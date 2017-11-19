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

#include "helpers.h"

#ifdef DAWFILTER_USEPYTHON
#include <boost/python.hpp>
#endif
#include <cstdint>
#include <string>
#include <tuple>

namespace daw {
	namespace imaging {
		template<typename T>
		struct GenericRGB final {
			T blue;
			T green;
			T red;

			constexpr GenericRGB( ) noexcept : blue{0}, green{0}, red{0} {}
			constexpr GenericRGB( T const &GS ) noexcept : blue{GS}, green{GS}, red{GS} {}

			constexpr GenericRGB( T const &Red, T const &Green, T const &Blue ) noexcept
			  : blue{Blue}, green{Green}, red{Red} {}

			constexpr void set_all( T const &Red, T const &Green, T const &Blue ) noexcept {
				blue = Blue;
				green = Green;
				red = Red;
			}

			constexpr void set_all( T const &grayscale ) noexcept {
				blue = grayscale;
				green = grayscale;
				red = grayscale;
			}

			constexpr GenericRGB( GenericRGB const & ) noexcept = default;
			constexpr GenericRGB( GenericRGB && ) noexcept = default;
			constexpr GenericRGB &operator=( GenericRGB const & ) noexcept = default;
			constexpr GenericRGB &operator=( GenericRGB && ) noexcept = default;

			~GenericRGB( ) noexcept = default;

			constexpr GenericRGB &operator=( T const &src ) noexcept {
				red = src;
				green = src;
				blue = src;
				return *this;
			}

			static constexpr float colform( GenericRGB<T> const &c, float Red, float Green, float Blue ) noexcept {
				return Red * static_cast<float>( c.red ) + Green * static_cast<float>( c.green ) +
				       Blue * static_cast<float>( c.blue );
			}

			constexpr float colform( float Red, float Green, float Blue ) const noexcept {
				return colform( *this, std::move( Red ), std::move( Green ), std::move( Blue ) );
			}

			constexpr void clampvalue( T const &min, T const &max ) noexcept {
				if( red < min ) {
					red = min;
				} else if( red > max ) {
					red = max;
				}
				if( green < min ) {
					green = min;
				} else if( green > max ) {
					green = max;
				}
				if( blue < min ) {
					blue = min;
				} else if( blue > max ) {
					blue = max;
				}
			}

			constexpr T min( ) const noexcept {
				T ret = red;
				if( green < ret ) {
					ret = green;
				}
				if( blue < ret ) {
					ret = blue;
				}
				return ret;
			}

			constexpr T max( ) const noexcept {
				T ret = red;
				if( green > ret ) {
					ret = green;
				}
				if( blue > ret ) {
					ret = blue;
				}
				return ret;
			}

			constexpr void mul( T const &value ) noexcept {
				blue *= value;
				green *= value;
				red *= value;
			}

			constexpr void div( T const &value ) noexcept {
				blue /= value;
				green /= value;
				red /= value;
			}

			constexpr float too_float_gs( ) const noexcept {
				return helpers::too_gs_small( red, green, blue );
			}

			template<typename V>
			constexpr GenericRGB<V> as( ) noexcept {
				return GenericRGB<V>( static_cast<V>( red ), static_cast<V>( green ), static_cast<V>( blue ) );
			}

#ifdef DAWFILTER_USEPYTHON
			static void register_python( std::string const &nameoftype ) {
				boost::python::class_<GenericRGB>( nameoftype.c_str( ), boost::python::init<>( ) )
				  .def( bpython::init<T, T, T>( ) )
				  .def_readwrite( "red", &GenericRGB::red )
				  .def_readwrite( "green", &GenericRGB::green )
				  .def_readwrite( "blue", &GenericRGB::blue );
			}
#endif
		};

		template<typename L, typename R>
		constexpr void min( GenericRGB<L> const &value, GenericRGB<R> &cur_min ) noexcept {
			if( value.red < cur_min.red ) {
				cur_min.red = value.red;
			}
			if( value.green < cur_min.green ) {
				cur_min.green = value.green;
			}
			if( value.blue < cur_min.blue ) {
				cur_min.blue = value.blue;
			}
		}

		template<typename L, typename R>
		constexpr void max( GenericRGB<L> const &value, GenericRGB<R> &cur_max ) noexcept {
			if( value.red > cur_max.red ) {
				cur_max.red = value.red;
			}
			if( value.green > cur_max.green ) {
				cur_max.green = value.green;
			}
			if( value.blue > cur_max.blue ) {
				cur_max.blue = value.blue;
			}
		}
	} // namespace imaging
} // namespace daw

using rgb3 = daw::imaging::GenericRGB<uint8_t>;
