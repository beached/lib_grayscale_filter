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

#pragma once

#ifdef DAWFILTER_USEPYTHON
#include <boost/python.hpp>
#endif

#include <boost/filesystem.hpp>
#include <memory>

#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <daw/daw_exception.h>
#include <daw/daw_random.h>
#include <daw/daw_string_view.h>

#include "fimage.h"
#include "genericrgb.h"

namespace daw {
	namespace imaging {
		template<class T>
		struct GenericImage {
			using value_type = ::std::decay_t<T>;
			using values_type = std::vector<value_type>;
			using iterator = typename values_type::iterator;
			using const_iterator = typename values_type::const_iterator;
			using reference = typename values_type::reference;
			using const_reference = typename values_type::const_reference;
			using id_t = uint32_t;

		private:
			size_t m_width;
			size_t m_height;
			size_t m_size;
			size_t m_id;
			values_type m_image_data;

		public:
			GenericImage( size_t width, size_t height )
			  : m_width{width}
			  , m_height{height}
			  , m_size{width * height}
			  , m_id{daw::randint<id_t>( )}
			  , m_image_data( width * height ) {}

			GenericImage( GenericImage const & ) = default;
			GenericImage( GenericImage && ) noexcept = default;
			GenericImage &operator=( GenericImage const & ) = default;
			GenericImage &operator=( GenericImage && ) noexcept = default;

			~GenericImage( ) = default;

			size_t width( ) const noexcept {
				return m_width;
			}

			size_t height( ) const noexcept {
				return m_height;
			}

			size_t size( ) const noexcept {
				return m_size;
			}

			size_t id( ) const noexcept {
				return m_id;
			}

			const_reference operator( )( size_t const row, size_t const col ) const {
				return arry( )[m_width * row + col];
			}

			reference operator( )( size_t const row, size_t const col ) {
				return arry( )[m_width * row + col];
			}

		private:
			values_type &arry( ) {
				return m_image_data;
			}

			values_type const &arry( ) const {
				return m_image_data;
			}

		public:
			const_reference operator[]( size_t const pos ) const {
				return arry( )[pos];
			}

			reference operator[]( size_t const pos ) {
				return arry( )[pos];
			}

			iterator begin( ) noexcept {
				return m_image_data.begin( );
			}

			const_iterator begin( ) const noexcept {
				return m_image_data.begin( );
			}

			const_iterator cbegin( ) const noexcept {
				return m_image_data.begin( );
			}

			iterator end( ) noexcept {
				return m_image_data.end( );
			}

			const_iterator end( ) const noexcept {
				return m_image_data.end( );
			}

			const_iterator cend( ) const noexcept {
				return m_image_data.end( );
			}

#ifdef DAWFILTER_USEPYTHON
			static void register_python( std::string const &nameoftype ) {
				boost::python::class_<GenericImage>( nameoftype.c_str( ), boost::python::init<size_t const, size_t const>( ) )
				  .def( "from_file", &GenericImage::from_file )
				  .static method( "from_file" )
				  .def( "to_file", &GenericImage::to_file )
				  .static method( "to_file" )
				  .add_property( "size", &GenericImage::size )
				  .add_property( "width", &GenericImage::width )
				  .add_property( "height", &GenericImage::height )
				  .add_property( "id", &GenericImage::id );
			}
#endif
		};

		template<>
		struct GenericImage<rgb3> {
			using value_type = rgb3;
			using values_type = std::vector<value_type>;
			using iterator = typename values_type::iterator;
			using const_iterator = typename values_type::const_iterator;
			using reference = typename values_type::reference;
			using const_reference = typename values_type::const_reference;
			using id_t = uint32_t;

		private:
			size_t m_width;
			size_t m_height;
			size_t m_size;
			size_t m_id;
			values_type m_image_data;

			inline values_type &arry( ) noexcept {
				return m_image_data;
			}

			inline values_type const &arry( ) const noexcept {
				return m_image_data;
			}

		public:
			GenericImage( size_t const width, size_t const height )
			  : m_width{width}
			  , m_height{height}
			  , m_size{width * height}
			  , m_id{daw::randint<id_t>( )}
			  , m_image_data( width * height ) {}

			GenericImage( GenericImage const & ) = default;
			GenericImage( GenericImage && ) noexcept = default;
			GenericImage &operator=( GenericImage && ) noexcept = default;
			GenericImage &operator=( GenericImage const & ) = default;

			~GenericImage( ) = default;

			static void to_file( daw::string_view image_filename, GenericImage<rgb3> const &image_input );

			inline void to_file( daw::string_view image_filename ) const {
				to_file( image_filename, *this );
			}

			static GenericImage<rgb3> from_file( daw::string_view image_filename );

			inline size_t width( ) const noexcept {
				return m_width;
			}

			inline size_t height( ) const noexcept {
				return m_height;
			}

			inline size_t size( ) const noexcept {
				return m_size;
			}

			inline size_t id( ) const noexcept {
				return m_id;
			}

			const_reference operator( )( size_t const y, size_t const x ) const {
				return arry( )[y * m_width + x];
			}

			reference operator( )( size_t const y, size_t const x ) {
				return arry( )[y * m_width + x];
			}

			const_reference operator[]( size_t const pos ) const {
				return arry( )[pos];
			}

			reference operator[]( size_t const pos ) {
				return arry( )[pos];
			}

			iterator begin( ) noexcept {
				return m_image_data.begin( );
			}

			const_iterator begin( ) const noexcept {
				return m_image_data.begin( );
			}

			const_iterator cbegin( ) const noexcept {
				return m_image_data.begin( );
			}

			iterator end( ) noexcept {
				return m_image_data.end( );
			}

			const_iterator end( ) const noexcept {
				return m_image_data.end( );
			}

			const_iterator cend( ) const noexcept {
				return m_image_data.end( );
			}
#ifdef DAWFILTER_USEPYTHON
			static void register_python( std::string const &nameoftype );
#endif
		};

		inline GenericImage<rgb3> from_file( daw::string_view image_filename ) {
			return GenericImage<rgb3>::from_file( image_filename );
		}
	} // namespace imaging
} // namespace daw
