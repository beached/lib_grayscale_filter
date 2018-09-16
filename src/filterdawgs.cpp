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
			GenericImage<rgb3> FilterDAWGS::filter( GenericImage<rgb3> const &input_image ) {

			std::vector<uint32_t> keys{};
			keys.resize( input_image.size( ) );
			// Create a unique_values item for each distinct grayscale item in image
			// and then set count to zero
			// no parallel to unique_values
			daw::algorithm::parallel::transform( input_image.begin( ), input_image.end( ), keys.begin( ), []( auto rgb ) {
				return daw::imaging::FilterDAWGS::too_gs( rgb );
			});

			// If we must compress as there isn't room for number of grayscale items
			if( keys.size( ) <= 256 ) {
				std::cerr << "Already a grayscale image or has enough room for all "
				             "possible values and no compression needed:"
				          << keys.size( ) << std::endl;
				GenericImage<rgb3> image_output{input_image.width( ), input_image.height( )};

				std::transform( input_image.cbegin( ), input_image.cend( ), image_output.begin( ),
				                []( rgb3 const &rgb ) { return static_cast<uint8_t>( rgb.too_float_gs( ) ); } );

				return image_output;
			}
			daw::algorithm::parallel::sort( keys.begin(), keys.end( ) );
			keys.erase( std::unique( keys.begin( ), keys.end( ) ), keys.end( ) );

			auto const inc = static_cast<float>( keys.size( ) ) / 256.0f;

			std::unordered_map<uint32_t, uint8_t> value_pos{};
			value_pos.reserve( keys.size( ) );

			for( size_t n = 0; n < keys.size( ); ++n ) {
				value_pos.emplace( keys[n], static_cast<uint8_t>( static_cast<float>( n ) / inc ) );
			}

			GenericImage<rgb3> output_image{input_image.width( ), input_image.height( )};

			daw::algorithm::parallel::transform( input_image.cbegin( ), input_image.cend( ), output_image.begin( ),
			                [&value_pos]( auto rgb ) { return value_pos[FilterDAWGS::too_gs( rgb )]; } );

			return output_image;
		}

#ifdef DAWFILTER_USEPYTHON
		void FilterDAWGS::register_python( std::string const nameoftype ) {
			boost::python::def( nameoftype.c_str( ), &FilterDAWGS::filter );
		}
#endif
	} // namespace imaging
} // namespace daw
