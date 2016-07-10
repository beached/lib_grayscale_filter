// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "filterdawgs2.h"
#include "genericimage.h"
#include "genericrgb.h"
#include <daw/daw_parallel_algorithm.h>
#include <daw/daw_array.h>
#include <daw/daw_utility.h>
#include <daw/daw_math.h>
#include <boost/scoped_array.hpp>
#include <unordered_map>
#include <iterator>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>
#include <cmath>

namespace daw {
	namespace imaging {
		namespace {
			template<typename Map>
			auto get_keys( Map const & m ) {
				daw::array<typename Map::key_type> result( m.size( ) );
				daw::algorithm::parallel::non::transform( m.begin( ), m.end( ), result.begin( ), []( auto const & val ) {
					return val.first;
				} );
				return result;
			}

			
			template<typename T>
			constexpr T const const_under_half = T(0.49999999999999999999999999999999999999999999999999999999999999999999999999999999);

			namespace impl {
				namespace {
					template<typename T>
					constexpr auto const & coefficients( ) noexcept {
						return []( ) {
							auto const sqrt_tmp = sqrt( static_cast<T>(0.125) );
							auto const pi_over_64 = daw::math::PI<T> / static_cast<T>(64.0);
							std::array<T, 64> result;
							for( size_t j = 0; j < 8; ++j ) {
								result[j] = sqrt_tmp;
								for( size_t i = 8; i < 64; i+=8 ) {
									auto rad = static_cast<T>(i)
										* (static_cast<T>(j) + static_cast<T>(0.5))
										* pi_over_64;
									result[i + j] = static_cast<T>(0.5) * daw::math::cos( rad );	
								}
							}
							return result;
						}( );
					}
				}	// namespace anonymous

				template<typename T, typename U>
				void coeffs( T & Cu, T & Cv, U u, U v ) {
					Cu = u == 0 ? 1.0/sqrt( 2.0 ) : 1.0;
					Cv = v == 0 ? 1.0/sqrt( 2.0 ) : 1.0;
				}
			}	// namespace impl


			template<typename T>
			GenericImage<double> dct( GenericImage<T> const & image, size_t const xpos, size_t const ypos ) {
				GenericImage<double> data( 8, 8 );
				for( size_t v=0; v<8; v++ ) {
					for( size_t u=0; u<8; u++ ) {
						double Cu = 0;
						double Cv = 0;
						double z = 0.0;

						impl::coeffs( Cu, Cv, u, v );

						for( size_t y=0; y<8; y++ ) {
							for( size_t x=0; x<8; x++ ) {
								auto s = static_cast<double>(image( x + xpos, y + ypos ));

								auto q = s * cos( static_cast<double>((2*x+1)*u) * daw::math::PI<double>/16.0 )
										* cos( static_cast<double>((2*y+1)*v) * daw::math::PI<double>/16.0 );
								z += q;
							}
						}
						data( v, u ) = 0.25 * Cu * Cv * z;
					}
				}
				return data;
			}

			template<typename T>
			void idct( GenericImage<T> & image, GenericImage<double> const & dct_data, size_t const xpos, size_t const ypos ) {
				// iDCT 
				for( size_t y=0; y<8; y++ ) {
					for( size_t x=0; x<8; x++ ) {
						double z = 0.0;

						for( size_t v=0; v<8; v++ ) {
							for( size_t u=0; u<8; u++ ) {
								double Cu = 0.0;
								double Cv = 0.0;
								
								impl::coeffs( Cu, Cv, u, v );
								auto S = dct_data( v, u );

								auto q = Cu * Cv * S *
									cos( static_cast<double>((2*x+1)*u) * daw::math::PI<double>/16.0 ) *
									cos( static_cast<double>((2*y+1)*v) * daw::math::PI<double>/16.0 );
								z += q;
							}
						}

						z /= 4.0;
						if( z > std::numeric_limits<T>::max( ) ) {
							image( x + xpos, y + ypos ) = std::numeric_limits<T>::max( );
						} else if( z < 0 ) {
							image( x + xpos, y + ypos ) = 0;
						} else {
							image( x + xpos, y + ypos ) = static_cast<T>( z );
						}
					}
				}
			}

		}	// namespace anonymous

		GenericImage<rgb3> FilterDAWGS2::filter( GenericImage<rgb3> const & image_input ) {
			GenericImage<double> result( daw::ceil_by( image_input.width( ), 8.0 ), daw::ceil_by( image_input.height( ), 8.0 ) );
			assert( image_input.size( ) == result.size( ) );
	
			// Expand GS and pad result image dimensions to next multiple of 8
			for( size_t y = 0; y < image_input.height( ); ++y ) {
				for( size_t x = 0; x < image_input.width( ); ++x ) {
					result( y, x ) = FilterDAWGS2::too_gs( image_input( y, x ) );
				}
				for( auto x = image_input.width( ); x<result.width( ); ++x ) {
					result( y, x ) = 0;
				}
			}
			for( auto y = image_input.height( ); y < result.height( ); ++y ) {
				for( size_t x = 0; x < result.width( ); ++x ) {
					result( y, x ) = 0;
				}
			}

			auto quantize = []( auto & dct_vals ) { 
				for( size_t i = 0; i < 8; ++i ) {
					for( size_t j = 0; j < 8; ++j ) {
						if( i > 3 || j > 3 ) {
							dct_vals( i, j ) = 0.0;
						}
					}				
				}
			};

			for( size_t y = 0; y < result.height( ); y += 8 ) {
				for( size_t x = 0; x < result.width( ); x+=8 ) {
					auto d = dct( result, x, y );
					quantize( d );
					// reduce gs values
					idct( result, d, x, y );
				}
			}

			GenericImage<rgb3> image_output( image_input.width( ), image_input.height( ) );
		
			for( size_t y = 0; y < result.height( ); ++y ) {
				for( size_t x = 0; x < result.width( ); ++x ) {
					auto c = static_cast<uint8_t>(result( x, y ));
					image_output( x, y ) = { c, c, c };
				}
			}	

			return image_output;
		}

		double FilterDAWGS2::too_gs( rgb3 const & pixel ) {
			// Returns a ~24bit grayscale value from 0 to ~16 million
			return static_cast<double>(19595 * pixel.red + 38469 * pixel.green + 7471 * pixel.blue)/65535.0;	// 0.299r + 0.587g + 0.114b
		}

#ifdef DAWFILTER_USEPYTHON
		void FilterDAWGS2::register_python( std::string const nameoftype ) {
			boost::python::def( nameoftype.c_str( ), &FilterDAWGS2::filter );
		}
#endif
	}	// namespace imaging
}	// namespace daw

