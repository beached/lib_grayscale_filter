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

#include <FreeImage.h>

#include <daw/daw_string_view.h>

namespace daw {
	namespace imaging {
		class FreeImage final {
			FIBITMAP *m_bitmap;

		public:
			using pos_t = decltype( FreeImage_GetWidth( m_bitmap ) );
			using bpp_t = decltype( FreeImage_GetBPP( m_bitmap ) );

			FreeImage( ) = delete;

			constexpr FreeImage( FIBITMAP *bitmap ) : m_bitmap{bitmap} {
				daw::exception::daw_throw_on_null( m_bitmap, "Error while loading FreeImage bitmap" );
			}

			inline FreeImage( FIBITMAP *bitmap, daw::string_view errmsg ) : m_bitmap{bitmap} {
				daw::exception::daw_throw_on_null( m_bitmap, errmsg.data( ) );
			}

			inline ~FreeImage( ) noexcept {
				this->close( );
			}

			inline FreeImage( FreeImage const &other ): m_bitmap{ FreeImage_Clone( other.m_bitmap ) } {
				daw::exception::daw_throw_on_null( other.m_bitmap, "Error while loading FreeImage bitmap" );
			}


			inline FreeImage &operator=( FreeImage const &rhs ) {
				if( this != &rhs ) {
					daw::exception::daw_throw_on_null( m_bitmap, "Error while loading FreeImage bitmap" );
					m_bitmap = FreeImage_Clone( rhs.m_bitmap );
				}
				return *this;
			}

			constexpr FreeImage( FreeImage &&other ) noexcept : m_bitmap{daw::exchange( other.m_bitmap, nullptr )} {}

			inline FreeImage &operator=( FreeImage && rhs ) noexcept {
				if( this != &rhs ) {
					this->close( );
					m_bitmap = daw::exchange( rhs.m_bitmap, nullptr );
				}
				return *this;
			}

			inline FreeImage &take( FreeImage &other ) {
				if( this != &other ) {
					daw::exception::daw_throw_on_null( other.m_bitmap, "Error, attempt to take ownership from a null FreImage" );
					m_bitmap = daw::exchange( other.m_bitmap, nullptr );
				}
				return *this;
			}

			inline FreeImage &take( FIBITMAP *bitmap ) {
				if( m_bitmap != bitmap ) {
					daw::exception::daw_throw_on_null( bitmap, "Error, attempt to take ownership from a null FreImage" );
					m_bitmap = bitmap;
				}
				return *this;
			}

			inline void close( ) noexcept {
				if( nullptr != m_bitmap ) {
					FreeImage_Unload( m_bitmap );
					m_bitmap = nullptr;
				}
			}

			constexpr FIBITMAP *ptr( ) noexcept {
				return m_bitmap;
			}

			constexpr FIBITMAP const *ptr( ) const noexcept {
				return m_bitmap;
			}

			inline pos_t height( ) const noexcept {
				return FreeImage_GetHeight( m_bitmap );
			}

			inline pos_t width( ) const noexcept {
				return FreeImage_GetWidth( m_bitmap );
			}

			inline bpp_t bpp( ) const noexcept {
				return FreeImage_GetBPP( m_bitmap );
			}
		};
	} // namespace imaging
} // namespace daw
