// The MIT License (MIT)
//
// Copyright (c) 2017 Darrell Wright
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

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>

#include <daw/daw_algorithm.h>
#include <daw/daw_benchmark.h>
#include <daw/daw_container_algorithm.h>
#include <daw/daw_exception.h>

#include "filterdawgs.h"

int main( int argc, char **argv ) {
	daw::exception::daw_throw_on_false( argc >= 2, "Must supply a source file" );
	auto const input_image = daw::imaging::from_file( argv[1] );
	auto const t1 = daw::benchmark( [img_ref = std::cref( input_image )]( ) {
		auto const &img = img_ref.get( );
		std::vector<uint32_t> valuepos{};
		valuepos.reserve( img.size( ) );
		daw::container::transform( img, std::back_inserter( valuepos ), []( auto rgb_val ) noexcept {
			return daw::imaging::FilterDAWGS::too_gs( rgb_val );
		} );

		std::sort( valuepos.begin( ), valuepos.end( ) );
		valuepos.erase( std::unique( valuepos.begin( ), valuepos.end( ) ), valuepos.end( ) );

		std::unordered_map<uint32_t, uint32_t> mv{};
		mv.reserve( valuepos.size( ) );
		auto const inc = static_cast<float>( valuepos.size( ) ) / 256.0f;

		for( size_t n = 0; n < valuepos.size( ); ++n ) {
			mv.emplace( valuepos[n], static_cast<float>( n ) / inc );
		}
		daw::do_not_optimize( mv );
	} );

	std::cout << "elapsed time method 1: " << daw::utility::format_seconds( t1, 2 ) << '\n';

	auto const t2 = daw::benchmark( [img_ref = std::cref( input_image )]( ) {
		// no parallel to valuepos
		auto const &image_input = img_ref.get( );
		std::unordered_set<uint32_t> valuepos{};

		// Create a valuepos item for each distinct grayscale item in image
		// and then set count to zero
		// no parallel to valuepos
		for( auto &rgb : image_input ) {
			valuepos.insert( daw::imaging::FilterDAWGS::too_gs( rgb ) );
		}
		std::vector<uint32_t> keys{};
		keys.reserve( valuepos.size( ) );
		std::copy( valuepos.cbegin( ), valuepos.cend( ), std::back_inserter( keys ) );
		std::sort( keys.begin( ), keys.end( ) );

		auto const inc = static_cast<float>( keys.size( ) ) / 256.0f;

		std::unordered_map<uint32_t, uint32_t> mv{};
		mv.reserve( keys.size( ) );

		for( size_t n = 0; n < keys.size( ); ++n ) {
			mv.emplace( keys[n], static_cast<uint32_t>( static_cast<float>( n ) / inc ) );
		}

		daw::do_not_optimize( mv );
	} );

	std::cout << "elapsed time method 2: " << daw::utility::format_seconds( t2, 2 ) << '\n';

	auto const t3 = daw::benchmark( [img_ref = std::cref( input_image )]( ) {
		// no parallel to valuepos
		auto const &image_input = img_ref.get( );

		std::set<uint32_t> valuepos{};

		// Create a valuepos item for each distinct grayscale item in image
		// and then set count to zero
		// no parallel to valuepos
		for( auto &rgb : image_input ) {
			valuepos.insert( daw::imaging::FilterDAWGS::too_gs( rgb ) );
		}

		auto const inc = static_cast<float>( valuepos.size( ) ) / 256.0f;

		std::unordered_map<uint32_t, uint32_t> mv{};
		mv.reserve( valuepos.size( ) );

		auto item = valuepos.begin( );
		for( size_t n = 0; n < valuepos.size( ); ++n ) {
			mv.emplace( *item, static_cast<uint32_t>( static_cast<float>( n ) / inc ) );
		}

		daw::do_not_optimize( mv );
	} );

	std::cout << "elapsed time method 3: " << daw::utility::format_seconds( t3, 2 ) << '\n';

	auto const t4 = daw::benchmark( [img_ref = std::cref( input_image )]( ) {
		// no parallel to valuepos
		auto const &image_input = img_ref.get( );

		std::set<uint32_t> valuepos{};

		// Create a valuepos item for each distinct grayscale item in image
		// and then set count to zero
		// no parallel to valuepos
		for( auto &rgb : image_input ) {
			valuepos.insert( daw::imaging::FilterDAWGS::too_gs( rgb ) );
		}

		auto const inc = static_cast<float>( valuepos.size( ) ) / 256.0f;

		std::unordered_map<uint32_t, uint32_t> mv{};
		mv.reserve( valuepos.size( ) );

		auto item = valuepos.begin( );
		for( size_t n = 0; n < valuepos.size( ); ++n ) {
			mv.emplace( *item, static_cast<uint32_t>( static_cast<float>( n ) / inc ) );
		}

		daw::do_not_optimize( mv );
	} );

	std::cout << "elapsed time method 4: " << daw::utility::format_seconds( t4, 2 ) << '\n';

	return EXIT_SUCCESS;
}

