// The MIT License (MIT)
//
// Copyright (c) 2016 Darrell Wright
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
namespace impl {
template <typename T> struct fixed_array_t {
  using value_type = T;
  using iterator = value_type *;
  using const_iterator = value_type const *;
  using reference = value_type &;
  using const_reference = value_type const &;
  using size_type = size_t;

private:
  value_type *m_values;
  size_type m_size;

public:
  fixed_array_t() = delete;

  fixed_array_t(size_t size)
      : m_values{new T[size]}, m_size{nullptr == m_values ? 0 : size} {}

  ~fixed_array_t() {
    if (nullptr != m_values) {
      delete[] m_values;
      m_values = nullptr;
    }
    m_size = 0;
  }

  fixed_array_t(fixed_array_t &&other) noexcept
      : m_values{std::exchange(other.m_values, nullptr)}, m_size{std::exchange(
                                                              other.m_size,
                                                              0)} {}

  fixed_array_t(fixed_array_t const &other)
      : m_values{other.m_size > 0 ? new T[other.m_size] : nullptr},
        m_size{other.m_size} {

    std::copy_n(other.m_values, other.m_size, m_values);
  }

  friend void swap(fixed_array_t &lhs, fixed_array_t &rhs) noexcept {
    using std::swap;
    swap(lhs.m_values, rhs.m_values);
    swap(lhs.m_size, rhs.m_size);
  }

  fixed_array_t &operator=(fixed_array_t &&rhs) noexcept {
    if (this != &rhs) {
      fixed_array_t tmp{std::move(rhs)};
      using std::swap;
      swap(*this, tmp);
    }
    return *this;
  }

  fixed_array_t &operator=(fixed_array_t const &rhs) {
    if (this != &rhs) {
      fixed_array_t tmp{rhs};
      using std::swap;
      swap(*this, tmp);
    }
    return *this;
  }

  explicit operator bool() const noexcept { return nullptr != m_values; }

  size_type size() const noexcept { return m_size; }

  reference operator[](size_type pos) noexcept { return *(m_values + pos); }

  const_reference operator[](size_type pos) const noexcept {
    return *(m_values + pos);
  }

  iterator data() noexcept { return m_values; }

  const_iterator data() const noexcept { return m_values; }

  iterator begin() noexcept { return m_values; }

  const_iterator begin() const noexcept { return m_values; }

  iterator end() noexcept { return m_values + m_size; }

  const_iterator end() const noexcept { return m_values + m_size; }

  reference front() { return *m_values; }

  const_reference front() const { return *m_values; }

  reference back() { return *(m_values + m_size - 1); }

  const_reference back() const { return *(m_values + m_size - 1); }

}; // fixed_array_t
} // namespace impl
template <class T> struct GenericImage {
  using value_type = ::std::decay_t<T>;
  using values_type = impl::fixed_array_t<value_type>;
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
  GenericImage(size_t width, size_t height)
      : m_width{width}, m_height{height}, m_size{width * height},
        m_id{daw::randint<id_t>()}, m_image_data(width * height) {}

  GenericImage(GenericImage const &other)
      : m_width{other.m_width}, m_height{other.m_height}, m_size{other.m_size},
        m_id{daw::randint<id_t>()}, m_image_data(other.m_size) {

    std::copy_n(other.m_image_data.begin(), other.m_size, m_image_data.begin());
  }

  GenericImage(GenericImage &&) = default;
  GenericImage &operator=(GenericImage &&) = default;

  friend void swap(GenericImage &lhs, GenericImage &rhs) noexcept {
    using std::swap;
    swap(lhs.m_width, rhs.m_width);
    swap(lhs.m_height, rhs.m_height);
    swap(lhs.m_size, rhs.m_size);
    swap(lhs.m_id, rhs.m_id);
    swap(lhs.m_image_data, rhs.m_image_data);
  }

  GenericImage &operator=(GenericImage const &rhs) {
    if (this != &rhs) {
      GenericImage tmp{rhs};
      using std::swap;
      swap(*this, tmp);
    }
    return *this;
  }

  virtual ~GenericImage() = default;

  size_t width() const { return m_width; }

  size_t height() const { return m_height; }

  size_t size() const { return m_size; }

  size_t id() const { return m_id; }

  const_reference operator()(size_t const row, size_t const col) const {
    return arry()[m_width * row + col];
  }

  reference operator()(size_t const row, size_t const col) {
    return arry()[m_width * row + col];
  }

private:
  values_type &arry() { return m_image_data; }

  values_type const &arry() const { return m_image_data; }

public:
  const_reference operator[](size_t const pos) const { return arry()[pos]; }

  reference operator[](size_t const pos) { return arry()[pos]; }

  iterator begin() { return m_image_data.begin(); }

  const_iterator begin() const { return m_image_data.begin(); }

  iterator end() { return m_image_data.end(); }

  const_iterator end() const { return m_image_data.end(); }

#ifdef DAWFILTER_USEPYTHON
  static void register_python(std::string const &nameoftype) {
    boost::python::class_<GenericImage>(
        nameoftype.c_str(), boost::python::init<size_t const, size_t const>())
        .def("from_file", &GenericImage::from_file)
        .static method("from_file")
        .def("to_file", &GenericImage::to_file)
        .static method("to_file")
        .add_property("size", &GenericImage::size)
        .add_property("width", &GenericImage::width)
        .add_property("height", &GenericImage::height)
        .add_property("id", &GenericImage::id);
  }
#endif
};

void swap(GenericImage<rgb3> &lhs, GenericImage<rgb3> &rhs) noexcept;

template <> struct GenericImage<rgb3> {
  using value_type = rgb3;
  using values_type = impl::fixed_array_t<value_type>;
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
  values_type &arry();
  values_type const &arry() const;

public:
  GenericImage(size_t const width, size_t const height);

  GenericImage(GenericImage const &other);
  GenericImage(GenericImage &&) = default;
  GenericImage &operator=(GenericImage &&) = default;

  friend void swap(GenericImage<rgb3> &lhs, GenericImage<rgb3> &rhs) noexcept;

  GenericImage &operator=(GenericImage const &rhs);
  virtual ~GenericImage();

  GenericImage view(size_t origin_x, size_t origin_y, size_t width,
                    size_t height);

  static void to_file(daw::string_view image_filename,
                      GenericImage<rgb3> const &image_input);
  void to_file(daw::string_view image_filename) const;
  static GenericImage<rgb3> from_file(daw::string_view image_filename);
  size_t width() const;
  size_t height() const;
  size_t size() const;
  size_t id() const;
  const_reference operator()(size_t const y, size_t const x) const;
  reference operator()(size_t const y, size_t const x);
  const_reference operator[](size_t const pos) const;
  reference operator[](size_t const pos);
  iterator begin();
  const_iterator begin() const;
  iterator end();
  const_iterator end() const;
#ifdef DAWFILTER_USEPYTHON
  static void register_python(std::string const &nameoftype);
#endif
};
GenericImage<rgb3> from_file(daw::string_view image_filename);
} // namespace imaging
} // namespace daw
