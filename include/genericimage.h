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


#pragma once

#include "genericrgb.h"
#include "random.h"


#ifdef DAWFILTER_USEPYTHON
#include <boost/python.hpp>
#endif

#include <memory>
#include <boost/filesystem.hpp>

#include <stdexcept>
#include <string>
#include <vector>
#include "fimage.h"
#include <boost/utility/string_ref.hpp>
#include <daw/daw_exception.h>

namespace daw {
	namespace imaging {
		template<class T>
		struct GenericImage {
			using value_type = ::std::decay_t<T>;
		private:
			using values_type_inner = std::vector<value_type>;
		public:
			using values_type = std::shared_ptr<values_type_inner>;
			using iterator = typename values_type_inner::iterator;
			using const_iterator = typename values_type_inner::const_iterator;
			using reference = typename values_type_inner::reference;
			using const_reference = typename values_type_inner::const_reference;
			using id_t = uint32_t;
		private:
			size_t m_width;
			size_t m_height;
			size_t m_size;
			size_t m_id;
			values_type m_image_data;

		public:
			GenericImage( size_t width, size_t height ): 
				m_width{ width }, 
				m_height{ height }, 
				m_size{ width*height }, 
				m_id{ Random<id_t>::getNext( ) }, 
				m_image_data{ std::make_shared<values_type_inner>( static_cast<size_t>( width*height ) ) } {

				daw::exception::daw_throw_on_null( m_image_data.get( ), "Error creating GenericImage" );
			}

			size_t width( ) const {
				return m_width;
			}

			size_t height( ) const {
				return m_height;
			}

			size_t size( ) const {
				return m_size;
			}

			size_t id( ) const {
				return m_id;
			}

			const_reference operator( )( size_t const row, size_t const col ) const {
				return arry( )[m_width*row + col];
			}

			reference operator( )( size_t const row, size_t const col ) {
				return arry( )[m_width*row + col];
			}

		private:
			values_type_inner & arry( ) {
				return *m_image_data;
			}

			values_type_inner const & arry( ) const {
				return *m_image_data;
			}
		public:

			const_reference operator[]( size_t const pos ) const {
				return arry( )[pos];
			}

			reference operator[]( size_t const pos ) {
				return arry( )[pos];
			}

			iterator begin( ) {
				return m_image_data->begin( );
			}

			const_iterator begin( ) const {
				return m_image_data->begin( );
			}

			iterator end( ) {
				return m_image_data->end( );
			}
			
			const_iterator end( ) const {
				return m_image_data->end( );
			}
			
#ifdef DAWFILTER_USEPYTHON
			static void register_python( std::string const & nameoftype ) {
				boost::python::class_<GenericImage>( nameoftype.c_str( ), boost::python::init<size_t const, size_t const>( ) )
					.def( "from_file", &GenericImage::from_file ).static method( "from_file" )
					.def( "to_file", &GenericImage::to_file ).static method( "to_file" )
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
		private:
			using values_type_inner = std::vector<value_type>;
		public:
			using values_type = std::shared_ptr<values_type_inner>;
			using iterator = typename values_type_inner::iterator;
			using const_iterator = typename values_type_inner::const_iterator;
			using reference = typename values_type_inner::reference;
			using const_reference = typename values_type_inner::const_reference;
			using id_t = uint32_t;
		private:
			size_t m_width;
			size_t m_height;
			size_t m_size;
			size_t m_id;
			values_type m_image_data;
			values_type_inner & arry( );
			values_type_inner const & arry( ) const;
		public:
			GenericImage( size_t const width, size_t const height );

			GenericImage view( size_t origin_x, size_t origin_y, size_t width, size_t height );

			static void to_file( boost::string_ref image_filename, GenericImage<rgb3> const& image_input );
			void to_file( boost::string_ref image_filename ) const;
			static GenericImage<rgb3> from_file( boost::string_ref image_filename );
			size_t width( ) const;
			size_t height( ) const;
			size_t size( ) const;
			size_t id( ) const;
			const_reference operator( )( size_t const y, size_t const x ) const;
			reference operator( )( size_t const y, size_t const x );
			const_reference operator[]( size_t const pos ) const;
			reference operator[]( size_t const pos );
			iterator begin( );
			const_iterator begin( ) const;
			iterator end( );
			const_iterator end( ) const;
#ifdef DAWFILTER_USEPYTHON
			static void register_python( std::string const& nameoftype );
#endif
		};
		GenericImage<rgb3> from_file( boost::string_ref image_filename );
	}	// namespace imaging
}	// namespace daw
