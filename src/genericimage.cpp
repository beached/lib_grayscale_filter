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

#include <daw/daw_exception.h>
#include <daw/daw_random.h>
#include <daw/daw_string_view.h>

#include "genericimage.h"

namespace daw {
	namespace imaging {
		GenericImage<rgb3>::values_type &GenericImage<rgb3>::arry( ) {
			return m_image_data;
		}

		GenericImage<rgb3>::values_type const &GenericImage<rgb3>::arry( ) const {
			return m_image_data;
		}

		GenericImage<rgb3>::GenericImage( size_t const width, size_t const height )
		  : m_width( width )
		  , m_height( height )
		  , m_size( width * height )
		  , m_id( daw::randint<id_t>( ) )
		  , m_image_data( width * height ) {}

		GenericImage<rgb3>::GenericImage( GenericImage const &other )
		  : m_width{other.m_width}
		  , m_height{other.m_height}
		  , m_size{other.m_size}
		  , m_id{daw::randint<id_t>( )}
		  , m_image_data( m_size ) {

			std::copy_n( other.m_image_data.begin( ), other.m_size, m_image_data.begin( ) );
		}

		void swap( GenericImage<rgb3> &lhs, GenericImage<rgb3> &rhs ) noexcept {
			using std::swap;
			swap( lhs.m_width, rhs.m_width );
			swap( lhs.m_height, rhs.m_height );
			swap( lhs.m_size, rhs.m_size );
			swap( lhs.m_id, rhs.m_id );
			swap( lhs.m_image_data, rhs.m_image_data );
		}

		GenericImage<rgb3> &GenericImage<rgb3>::operator=( GenericImage<rgb3> const &rhs ) {
			if( this != &rhs ) {
				GenericImage tmp{rhs};
				using std::swap;
				swap( *this, tmp );
			}
			return *this;
		}

		GenericImage<rgb3>::~GenericImage( ) {}

		void GenericImage<rgb3>::to_file( daw::string_view image_filename, GenericImage<rgb3> const &image_input ) {
			try {
				daw::exception::daw_throw_on_false( image_input.width( ) <=
				                                    static_cast<size_t>( std::numeric_limits<int>::max( ) ) );
				daw::exception::daw_throw_on_false( image_input.height( ) <=
				                                    static_cast<size_t>( std::numeric_limits<int>::max( ) ) );
				FreeImage image_output( FreeImage_Allocate( static_cast<int>( image_input.width( ) ),
				                                            static_cast<int>( image_input.height( ) ), 24 ) );
				{
					daw::exception::daw_throw_on_false( image_input.height( ) > 0 );
					auto const maxy = image_input.height( ) - 1;
					//#pragma omp parallel for
					for( size_t y = 0; y < image_input.height( ); ++y ) {
						RGBQUAD rgb_out;
						for( size_t x = 0; x < image_input.width( ); ++x ) {
							rgb3 const rgb_in = image_input( y, x );
							rgb_out.rgbBlue = rgb_in.blue;
							rgb_out.rgbGreen = rgb_in.green;
							rgb_out.rgbRed = rgb_in.red;

							if( !FreeImage_SetPixelColor( image_output.ptr( ), static_cast<unsigned>( x ),
							                              static_cast<unsigned>( maxy - y ), &rgb_out ) ) {
								throw std::runtime_error( "Error setting pixel data" );
							}
						}
					}
				}
				auto fif = FreeImage_GetFIFFromFilename( image_filename.data( ) );
				if( !FreeImage_Save( fif, image_output.ptr( ), image_filename.data( ) ) ) {
					auto const msg = "Error Saving image to file '" + image_filename.to_string( ) + "'";
					throw std::runtime_error( msg );
				}
				image_output.close( );
			} catch( std::runtime_error const & ) { throw; } catch( ... ) {
				auto const msg =
				  "An unknown exception has been thrown while saving image to file '" + image_filename.to_string( ) + "'";
				throw std::runtime_error( msg );
			}
		}

		void GenericImage<rgb3>::to_file( daw::string_view image_filename ) const {
			GenericImage<rgb3>::to_file( image_filename, *this );
		}

		GenericImage<rgb3> GenericImage<rgb3>::from_file( daw::string_view image_filename ) {
			try {
				{
					boost::filesystem::path const pImageFile( image_filename.data( ) );
					if( !boost::filesystem::exists( pImageFile ) ) {
						auto const msg = "The file '" + image_filename.to_string( ) + "' cannot be found";
						throw std::runtime_error( msg );
					} else if( !boost::filesystem::is_regular_file( pImageFile ) ) {
						auto const msg = "The file '" + image_filename.to_string( ) + "' is not a regular file";
						throw std::runtime_error( msg );
					}
				}

				// Get Image Format and open the image
				auto fif = FreeImage_GetFileType( image_filename.data( ) );
				if( fif == FIF_UNKNOWN ) {
					fif = FreeImage_GetFIFFromFilename( image_filename.data( ) );
					if( fif == FIF_UNKNOWN ) {
						auto const msg =
						  "The file '" + image_filename.to_string( ) + "' cannot be opened.  Cannot determine image type";
						throw std::runtime_error( msg );
					}
				}

				std::string const input_image_msg = "Could not open input image '" + image_filename + '\'';
				FreeImage image_input( FreeImage_Load( fif, image_filename.data( ) ), input_image_msg.c_str( ) );

				if( !( image_input.bpp( ) == 24 || image_input.bpp( ) == 32 ) ||
				    FreeImage_GetColorType( image_input.ptr( ) ) != FIC_RGB ) {
					FIBITMAP *bitmap_test = nullptr;
					bitmap_test = FreeImage_ConvertTo24Bits( image_input.ptr( ) );
					if( nullptr == bitmap_test ) {
						bitmap_test = FreeImage_ConvertTo32Bits( image_input.ptr( ) );
						if( nullptr == bitmap_test ) {
							auto const msg = "'" + image_filename.to_string( ) + "' is a non RGB8 file.  Files must be RGB8";
							throw std::runtime_error( msg );
						} else {
							std::cerr << "Had to convert image to 32bit RGBA" << std::endl;
						}
					} else {
						std::cerr << "Had to convert image to 24bit RGB" << std::endl;
					}
					image_input.take( bitmap_test );
				}
				GenericImage<rgb3> image_output( image_input.width( ), image_input.height( ) );

				{
					daw::exception::daw_throw_on_false( image_output.width( ) <=
					                                    static_cast<size_t>( std::numeric_limits<unsigned>::max( ) ) );
					daw::exception::daw_throw_on_false( image_output.height( ) <=
					                                    static_cast<size_t>( std::numeric_limits<unsigned>::max( ) ) );
					auto const maxy = image_output.height( ) - 1;

					for( size_t y = 0; y < image_output.height( ); ++y ) {
						RGBQUAD rgb_in;
						for( size_t x = 0; x < image_output.width( ); ++x ) {
							if( FreeImage_GetPixelColor( image_input.ptr( ), static_cast<unsigned>( x ),
							                             static_cast<unsigned>( maxy - y ), &rgb_in ) ) {
								rgb3 const rgb_out( rgb_in.rgbRed, rgb_in.rgbGreen, rgb_in.rgbBlue );
								image_output( y, x ) = rgb_out;
							} else {
								auto const msg = "Error retrieving pixel data from '" + image_filename.to_string( ) + "'";
								throw std::runtime_error( msg );
							}
						}
					}
				}
				image_input.close( );
				return image_output;
			} catch( std::runtime_error const & ) { throw; } catch( ... ) {
				auto const msg = "Unknown error while reading file'" + image_filename.to_string( ) + "'";
				throw std::runtime_error( msg );
			}
		}

		size_t GenericImage<rgb3>::width( ) const {
			return m_width;
		}

		size_t GenericImage<rgb3>::height( ) const {
			return m_height;
		}

		size_t GenericImage<rgb3>::size( ) const {
			return m_size;
		}

		size_t GenericImage<rgb3>::id( ) const {
			return m_id;
		}

		GenericImage<rgb3>::const_reference GenericImage<rgb3>::operator( )( size_t const y, size_t const x ) const {
			return arry( )[y * m_width + x];
		}

		GenericImage<rgb3>::reference GenericImage<rgb3>::operator( )( size_t const y, size_t const x ) {
			return arry( )[y * m_width + x];
		}

		GenericImage<rgb3>::const_reference GenericImage<rgb3>::operator[]( size_t const pos ) const {
			return arry( )[pos];
		}

		GenericImage<rgb3>::reference GenericImage<rgb3>::operator[]( size_t const pos ) {
			return arry( )[pos];
		}

		GenericImage<rgb3> from_file( daw::string_view image_filename ) {
			return GenericImage<rgb3>::from_file( image_filename );
		}

		GenericImage<rgb3>::iterator GenericImage<rgb3>::begin( ) {
			return m_image_data.begin( );
		}

		GenericImage<rgb3>::const_iterator GenericImage<rgb3>::begin( ) const {
			return m_image_data.begin( );
		}

		GenericImage<rgb3>::const_iterator GenericImage<rgb3>::cbegin( ) const {
			return m_image_data.begin( );
		}

		GenericImage<rgb3>::iterator GenericImage<rgb3>::end( ) {
			return m_image_data.end( );
		}

		GenericImage<rgb3>::const_iterator GenericImage<rgb3>::end( ) const {
			return m_image_data.end( );
		}

		GenericImage<rgb3>::const_iterator GenericImage<rgb3>::cend( ) const {
			return m_image_data.end( );
		}

#ifdef DAWFILTER_USEPYTHON
		void GenericImage<rgb3>::register_python( std::string const &nameoftype ) {
			boost::python::class_<GenericImage<rgb3>>( nameoftype.c_str( ),
			                                           boost::python::init<size_t const, size_t const>( ) )
			  .def( "from_file", &GenericImage<rgb3>::from_file )
			  .static method( "from_file" )
			  .def( "to_file", &GenericImage<rgb3>::to_file )
			  .static method( "to_file" )
			  .add_property( "size", &GenericImage<rgb3>::size )
			  .add_property( "width", &GenericImage<rgb3>::width )
			  .add_property( "height", &GenericImage<rgb3>::height )
			  .add_property( "id", &GenericImage<rgb3>::id );
		}
#endif
	} // namespace imaging
} // namespace daw
