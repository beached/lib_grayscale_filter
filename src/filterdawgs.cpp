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

#include <iostream>
#include <iterator>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <daw/daw_algorithm.h>
#include <daw/daw_array.h>
#include <daw/daw_container_algorithm.h>
#include <daw/fs/algorithms.h>

#include "filterdawgs.h"
#include "genericimage.h"
#include "genericrgb.h"

namespace daw {
	namespace imaging {
		namespace impl {
			GenericImage<rgb3> to_small_gs( GenericImage<rgb3> const &input_image ) {
				GenericImage<rgb3> image_output{input_image.width( ),
				                                input_image.height( )};

				std::transform( input_image.cbegin( ), input_image.cend( ),
				                image_output.begin( ), []( rgb3 const &rgb ) {
					                return static_cast<uint8_t>( rgb.too_float_gs( ) );
				                } );

				return image_output;
			}
		}
		GenericImage<rgb3>
		FilterDAWGS::filter( GenericImage<rgb3> const &input_image ) {

			std::vector<uint32_t> const keys = [&]( ) {
				std::vector<uint32_t> v{};
				v.resize( input_image.size( ) );

				daw::algorithm::parallel::transform(
				  input_image.begin( ), input_image.end( ), v.begin( ),
				  []( auto rgb ) { return daw::imaging::FilterDAWGS::too_gs( rgb ); } );

				daw::algorithm::parallel::sort( v.begin( ), v.end( ) );
				v.erase( std::unique( v.begin( ), v.end( ) ), v.end( ) );
				return v;
			}( );
			// If we must compress as there isn't room for number of grayscale items
			if( keys.size( ) <= 256 ) {
				std::cerr << "Already a grayscale image or has enough room for all "
										 "possible values and no compression needed:"
									<< keys.size( ) << std::endl;
				return impl::to_small_gs( input_image );
			}

			std::array<uint32_t, 256> bins = [&keys]( ) {
				std::array<uint32_t, 256> a{};
				auto const inc = static_cast<float>( keys.size( ) ) / 256.0f;
				for( size_t n=0; n<255; ++n ) {
					a[n] = keys[static_cast<size_t>(static_cast<float>(n)*inc)];
				}
				a[255] = keys.back( );	// Just in case
				return a;
			}( );
			GenericImage<rgb3> output_image{input_image.width( ),
			                                input_image.height( )};

			daw::algorithm::parallel::transform(
			  input_image.cbegin( ), input_image.cend( ), output_image.begin( ),
			  [&bins]( auto rgb ) -> uint8_t {
			  	auto const val = FilterDAWGS::too_gs( rgb );
			  	auto const pos = std::find_if( bins.begin( ), bins.end( ), [val]( auto b ) {
			  		return b >= val;
			  	} );
			  	return std::distance( bins.begin( ), pos );
			  } );

			return output_image;
		}

#ifdef DAWFILTER_USEPYTHON
		void FilterDAWGS::register_python( std::string const nameoftype ) {
			boost::python::def( nameoftype.c_str( ), &FilterDAWGS::filter );
		}
#endif
	} // namespace imaging
} // namespace daw
