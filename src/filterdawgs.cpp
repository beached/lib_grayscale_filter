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

#include "filterdawgs.h"
#include "genericimage.h"
#include "genericrgb.h"

namespace daw {
	namespace imaging {
		GenericImage<rgb3> FilterDAWGS::filter( GenericImage<rgb3> const &image_input ) {

			std::unordered_set<uint32_t> unique_values{};

			// Create a unique_values item for each distinct grayscale item in image
			// and then set count to zero
			// no parallel to unique_values
			for( auto const &rgb : image_input ) {
				unique_values.insert( daw::imaging::FilterDAWGS::too_gs( rgb ) );
			}

			// If we must compress as there isn't room for number of grayscale items
			if( unique_values.size( ) <= 256 ) {
				std::cerr << "Already a grayscale image or has enough room for all "
				             "possible values and no compression needed:"
				          << unique_values.size( ) << std::endl;
				GenericImage<rgb3> image_output{image_input.width( ), image_input.height( )};

				std::transform( image_input.cbegin( ), image_input.cend( ), image_output.begin( ),
				                []( rgb3 const &rgb ) { return static_cast<uint8_t>( rgb.too_float_gs( ) ); } );

				return image_output;
			}

			auto const keys = [&unique_values]( ) {
				std::vector<uint32_t> result{};
				result.reserve( unique_values.size( ) );
				std::copy( unique_values.cbegin( ), unique_values.cend( ), std::back_inserter( result ) );
				std::sort( result.begin( ), result.end( ) );
				return result;
			}( );

			auto const inc = static_cast<float>( keys.size( ) ) / 256.0f;

			std::unordered_map<uint32_t, uint8_t> value_pos{};
			value_pos.reserve( keys.size( ) );

			for( size_t n = 0; n < keys.size( ); ++n ) {
				value_pos.emplace( keys[n], static_cast<uint8_t>( static_cast<float>( n ) / inc ) );
			}

			GenericImage<rgb3> image_output{image_input.width( ), image_input.height( )};

			// TODO: make parallel
			std::transform( image_input.cbegin( ), image_input.cend( ), image_output.begin( ),
			                [&value_pos]( auto const &rgb ) { return value_pos[FilterDAWGS::too_gs( rgb )]; } );

			return image_output;
		}

#ifdef DAWFILTER_USEPYTHON
		void FilterDAWGS::register_python( std::string const nameoftype ) {
			boost::python::def( nameoftype.c_str( ), &FilterDAWGS::filter );
		}
#endif
	} // namespace imaging
} // namespace daw
