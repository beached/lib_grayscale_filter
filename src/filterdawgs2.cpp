// The MIT License (MIT)
//
// Copyright (c) 2016-2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <numeric>
#include <stdexcept>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include <daw/daw_array.h>
#include <daw/daw_container_algorithm.h>
#include <daw/daw_exception.h>
#include <daw/daw_math.h>
#include <daw/daw_static_array.h>
#include <daw/daw_utility.h>

#include "filterdawgs2.h"
#include "genericimage.h"
#include "genericrgb.h"

namespace daw {
	namespace imaging {
		namespace {
			template<typename Map>
			auto get_keys( Map const &m ) {
				std::vector<typename Map::key_type> result{};
				result.reserve( m.size( ) );
				daw::container::transform( m, std::back_inserter( result ), []( auto const &val ) { return val.first; } );
				return result;
			}

			template<typename T>
			constexpr T const
			  const_under_half = T( 0.49999999999999999999999999999999999999999999999999999999999999999999999999999999 );

			namespace impl {
				namespace {
					template<typename T>
					constexpr auto coefficients( ) noexcept {
						auto const sqrt_tmp = sqrt( static_cast<T>( 0.125 ) );
						auto const pi_over_64 = daw::math::PI<T> / static_cast<T>( 64.0 );
						daw::static_array_t<T, 64> result;
						for( size_t j = 0; j < 8; ++j ) {
							result[j] = sqrt_tmp;
							for( size_t i = 8; i < 64; i += 8 ) {
								auto rad = static_cast<T>( i ) * ( static_cast<T>( j ) + static_cast<T>( 0.5 ) ) * pi_over_64;
								result[i + j] = static_cast<T>( 0.5 ) * daw::math::cos( rad );
							}
						}
						return result;
					}
				} // namespace

				template<typename T, typename U>
				void coeffs( T &Cu, T &Cv, U u, U v ) {
					Cu = u == 0 ? 1.0 / sqrt( 2.0 ) : 1.0;
					Cv = v == 0 ? 1.0 / sqrt( 2.0 ) : 1.0;
				}
			} // namespace impl

			template<typename T>
			GenericImage<double> dct( GenericImage<T> const &image, size_t const xpos, size_t const ypos ) {
				GenericImage<double> data( 8, 8 );
				for( size_t v = 0; v < 8; v++ ) {
					for( size_t u = 0; u < 8; u++ ) {
						double Cu = 0;
						double Cv = 0;
						double z = 0.0;

						impl::coeffs( Cu, Cv, u, v );

						for( size_t y = 0; y < 8; y++ ) {
							for( size_t x = 0; x < 8; x++ ) {
								auto s = static_cast<double>( image( x + xpos, y + ypos ) );

								auto q = s * cos( static_cast<double>( ( 2 * x + 1 ) * u ) * daw::math::PI<double> / 16.0 ) *
								         cos( static_cast<double>( ( 2 * y + 1 ) * v ) * daw::math::PI<double> / 16.0 );
								z += q;
							}
						}
						data( v, u ) = 0.25 * Cu * Cv * z;
					}
				}
				return data;
			}

			template<typename T>
			void idct( GenericImage<T> &image, GenericImage<double> const &dct_data, size_t const xpos, size_t const ypos ) {
				// iDCT
				for( size_t y = 0; y < 8; y++ ) {
					for( size_t x = 0; x < 8; x++ ) {
						double z = 0.0;

						for( size_t v = 0; v < 8; v++ ) {
							for( size_t u = 0; u < 8; u++ ) {
								double Cu = 0.0;
								double Cv = 0.0;

								impl::coeffs( Cu, Cv, u, v );
								auto S = dct_data( v, u );

								auto q = Cu * Cv * S * cos( static_cast<double>( ( 2 * x + 1 ) * u ) * daw::math::PI<double> / 16.0 ) *
								         cos( static_cast<double>( ( 2 * y + 1 ) * v ) * daw::math::PI<double> / 16.0 );
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

		} // namespace

		GenericImage<rgb3> FilterDAWGS2::filter( GenericImage<rgb3> const &image_input ) {

			auto sum =
			  std::accumulate( image_input.begin( ), image_input.end( ), std::tuple<uintmax_t, uintmax_t, uintmax_t>{0, 0, 0},
			                   []( auto init, auto const &current ) {
				                   std::get<0>( init ) += current.red;
				                   std::get<1>( init ) += current.green;
				                   std::get<2>( init ) += current.blue;
				                   return std::move( init );
			                   } );

			std::get<0>( sum ) /= image_input.size( );
			std::get<1>( sum ) /= image_input.size( );
			std::get<2>( sum ) /= image_input.size( );

			GenericImage<rgb3> image_output( image_input.width( ), image_input.height( ) );

			auto mx = static_cast<double>( std::max( {std::get<0>( sum ), std::get<1>( sum ), std::get<2>( sum )} ) );
			auto weight_red = static_cast<double>( std::get<0>( sum ) ) / mx;
			auto weight_green = static_cast<double>( std::get<1>( sum ) ) / mx;
			auto weight_blue = static_cast<double>( std::get<2>( sum ) ) / mx;
			auto dv = ( weight_red + weight_green + weight_blue ) / 3.0;

			daw::exception::daw_throw_on_false( image_input.size( ) <= image_output.size( ) );
			// TODO: make parallel
			daw::container::transform( image_input, image_output.begin( ), [&]( rgb3 const &rgb ) {
				auto result = static_cast<uint8_t>(
				  ( ( static_cast<double>( rgb.red ) / weight_red + static_cast<double>( rgb.green ) / weight_green +
				      static_cast<double>( rgb.blue ) / weight_blue ) /
				    dv ) /
				  3.0 );
				return result;
			} );
			return image_output;
		}

#ifdef DAWFILTER_USEPYTHON
		void FilterDAWGS2::register_python( std::string const nameoftype ) {
			boost::python::def( nameoftype.c_str( ), &FilterDAWGS2::filter );
		}
#endif
	} // namespace imaging
} // namespace daw
