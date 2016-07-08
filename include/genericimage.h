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
#include "nullptr.h"
#include "random.h"


#ifdef DAWFILTER_USEPYTHON
#include <boost/python.hpp>
#endif

#include <boost/shared_array.hpp>
#include <boost/filesystem.hpp>

#include <stdexcept>
#include <string>
#include "fimage.h"
#include <boost/utility/string_ref.hpp>


namespace daw {
	namespace imaging {
		template<class T>
		struct GenericImage {
			using size_t = uint32_t;
			using value_type = T;
			using values_type = boost::shared_array<value_type>;
			using iterator = value_type *;
			using const_iterator = value_type const *;			
			using reference = value_type &;
			using const_reference = value_type const &;
		private:
			size_t m_width;
			size_t m_height;
			size_t m_row_width;
			size_t m_origin_x;
			size_t m_origin_y;
			size_t m_size;
			size_t m_id;
			boost::shared_array<T> m_image_data;

			GenericImage( size_t width, size_t height, size_t origin_x, size_t origin_y, size_t row_width, size_t image_size, boost::shared_array<T> image_data ): 
				m_width{ width }, 
				m_height{ height }, 
				m_origin_x{ origin_x },
				m_origin_y{ origin_y },
				m_row_width{ row_width },
				m_size{ image_size },
				m_id{ Random<size_t>::getNext( },
				m_image_data{ image_data } {

					assert( m_origin_x + m_width <= m_row_width );
					assert( m_row_width*height <= m_image_size ); 
				}
		public:
			GenericImage( size_t width, size_t height ): 
				m_width( width ), 
				m_height( height ), 
				m_origin_x{ 0 }, 
				m_origin_y{ 0 }, 
				m_row_width{ width }, 
				m_size( width*height ), 
				m_id( Random<size_t>::getNext( ) ), 
				m_image_data( new T[m_row_width*m_height] ) {

				nullcheck( m_image_data.get( ), "Error creating GenericImage" );
			}

			GenericImage view( size_t origin_x, size_t origin_y, size_t width, size_t height ) {
				return result( width, height, origin_x, origin_y, m_row_width, width*height, m_image_data );
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
				return m_image_data[(m_origin_y + row)*m_row_width + col + m_origin_x];
			}

			reference operator( )( size_t const row, size_t const col ) {
				return m_image_data[(m_origin_y + row)*m_row_width + col + m_origin_x];
			}

		private:
			constexpr size_t convert_pos( size_t pos ) const {
				auto y = pos / width;
				auto x = pos - (y*width);
				return (m_origin_y + y)*m_row_width + x + m_origin_x;
			}
		public:

			const_reference operator[]( size_t pos ) const {
				pos = convert_pos( );
				return m_image_data[pos];
			}

			reference operator[]( size_t pos ) {
				pos = convert_pos( );
				return m_image_data[pos];
			}

			iterator begin( ) {
				return m_image_data.get( );
			}

			const_iterator begin( ) const {
				return m_image_data.get( );
			}

			iterator end( ) {
				return m_image_data.get( ) + m_size;
			}
			const_iterator end( ) const {
				return m_image_data.get( ) + m_size;
			}

#ifdef DAWFILTER_USEPYTHON
			static void register_python( std::string const & nameoftype ) {
				boost::python::class_<GenericImage>( nameoftype.c_str( ), boost::python::init<const GenericImage::size_t, const GenericImage::size_t>( ) )
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
		class GenericImage<rgb3> {
		public:
			using value_type = rgb3;
			using values_type = boost::shared_array<value_type>;
			using iterator = value_type *;
			using const_iterator = value_type const *;
			using reference = value_type &;
			using const_reference = value_type const &;
		private:
			size_t m_width;
			size_t m_height;
			size_t m_row_width;
			size_t m_size;
			size_t m_id;
			values_type m_image_data;
			constexpr size_t convert_pos( size_t pos ) const;

			GenericImage( size_t width, size_t height, size_t origin_x, size_t origin_y, size_t row_width, size_t image_size, boost::shared_array<T> image_data ); 
		public:
			GenericImage( size_t const width, size_t const height );

			GenericImage view( size_t origin_x, size_t origin_y, size_t width, size_t height ) {
				return result( width, height, origin_x, origin_y, m_row_width, m_image_data );
			}

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
