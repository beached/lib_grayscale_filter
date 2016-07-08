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
			constexpr T const const_pi = T(3.14159265358979323846264338327950288419716939937510582097494459230781640628620899);

			template<typename T>
			constexpr T const const_under_half = T(0.49999999999999999999999999999999999999999999999999999999999999999999999999999999);

			template<typename T>
			constexpr T const coefficients = []( ) {
				T const sqrt_tmp = sqrt( static_cast<T>(0.125) );
				std::array<T, 64> result;
				for( size_t j = 0; j < 8; ++j ) {
					result[j] = sqrt_tmp;
					for( size_t i = 8; i < 64; i+=8 ) {
						result[i + j] = static_cast<T>(0.5) * cos( static_cast<T>(i) * (static_cast<T>(j) + static_cast<T>(0.5)) * const_pi<T> / static_cast<T>(64.0));	
					}
				}
				return result;
			}( );

			template<typename T = double>
			void forward_dct( GenericImage<uint32_t> & image ) {
				assert( image.width( ) >= 8 );
				assert( image.height( ) >= 8 );
				GenericImage<T> result( 8, 8 );	
				
				for( size_t i=0; i<64; i+=8 ) {
					for( size_t j=0; j<8; ++j ) {
						T tmp = 0;
						for( size_t k=0; k<8; ++k ) {
							tmp += coefficients<T>[i + k] * static_cast<T>(image[k * 8 + j]);
						}
						result[i + j] = tmp * static_cast<T>( 8 );
					}
				}

				for( size_t j=0; j<8; ++j ) {
					for( size_t i=0; i<64; i+=8 ) {
						T tmp = 0;
						for( size_t k=0; k<8; ++k ) {
							tmp += result[i + k] * coefficients<T>[j * 8 + k];
						}
						image[i + j] = floor( tmp + const_under_half<T> );
					}
				}
			}

		}	// namespace anonymous

		GenericImage<rgb3> FilterDAWGS2::filter( GenericImage<rgb3> const & image_input ) {
			GenericImage<int32_t> result( daw::ceil_by( image_input.width( ), 8.0 ), daw::ceil_by( image_input.height( ), 8.0 ) );
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

			for( size_t y = 0; y < result.height( ); y += 8 ) {
				for( size_t x = 0; x < result.width( ); x+=8 ) {
					auto current_view = result.view( x, y, 8, 8 );
					forward_dct<double>( current_view );
				}
			}

			/*if( valuepos.size( ) > 256 ) {
				auto const inc = static_cast<float>( valuepos.size( ) ) / 256.0f;
				{
					auto keys = get_keys( valuepos );

					std::sort( keys.begin( ), keys.end( ) );

					daw::algorithm::parallel::for_each( 0, keys.size( ), [&]( auto n ) {
						auto const curval = static_cast<int32_t>(static_cast<float>(n) / inc);
						auto const curkey = static_cast<int32_t>(keys[n]);	// TODO clarify why sign changes
						valuepos[curkey] = curval;
					} );
				}

				GenericImage<rgb3> image_output( image_input.width( ), image_input.height( ) );

				// TODO: make parallel
				daw::algorithm::parallel::non::transform( image_input.begin( ), image_input.end( ), image_output.begin( ), [&vp=valuepos]( rgb3 const & rgb ) {
					return static_cast<uint8_t>(vp[FilterDAWGS2::too_gs( rgb )]);
				} );
				return image_output;
			} else { // Already a grayscale image or has enough room for all possible values and no compression needed
				std::cerr << "Already a grayscale image or has enough room for all possible values and no compression needed:" << valuepos.size( ) << std::endl;
				GenericImage<rgb3> image_output( image_input.width( ), image_input.height( ) );

				assert( image_input.size( ) <= image_output.size( ) );
				// TODO: make parallel
				daw::algorithm::parallel::non::transform( image_input.begin( ), image_input.end( ), image_output.begin( ), []( rgb3 const & rgb ) {
					return static_cast<uint8_t>(rgb.too_float_gs( ));
				} );
				return image_output;
			}*/
			GenericImage<rgb3> image_output( image_input.width( ), image_input.height( ) );
			return image_output;
		}

		int32_t FilterDAWGS2::too_gs( rgb3 const & pixel ) {
			// Returns a ~24bit grayscale value from 0 to ~16 million
			return 19595 * pixel.red + 38469 * pixel.green + 7471 * pixel.blue;	// 0.299r + 0.587g + 0.114b
		}

#ifdef DAWFILTER_USEPYTHON
		void FilterDAWGS2::register_python( std::string const nameoftype ) {
			boost::python::def( nameoftype.c_str( ), &FilterDAWGS2::filter );
		}
#endif
	}	// namespace imaging
}	// namespace daw

