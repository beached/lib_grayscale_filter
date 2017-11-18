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

#include <boost/scoped_array.hpp>
#include <iterator>
#include <map>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

#include <daw/daw_algorithm.h>
#include <daw/daw_array.h>
#include <daw/daw_container_algorithm.h>

#include "filterdawgs.h"
#include "genericimage.h"
#include "genericrgb.h"

namespace daw {
namespace imaging {
namespace {
template <typename Map> auto get_keys(Map const &m) {
  std::vector<typename Map::key_type> result{};
  result.reserve(m.size());
  daw::container::transform(m, std::back_inserter(result),
                            [](auto const &val) { return val.first; });
  return result;
}
} // namespace
GenericImage<rgb3> FilterDAWGS::filter(GenericImage<rgb3> const &image_input) {
  // no parallel to valuepos
  using valuepos_t = std::unordered_map<int32_t, int32_t>;
  valuepos_t valuepos{};

	// Create a valuepos item for each distinct grayscale item in image
	// and then set count to zero
  // no parallel to valuepos
  for (auto &rgb : image_input) {
    valuepos[FilterDAWGS::too_gs(rgb)] = 0;
  }

	// If we must compress as there isn't room for number of grayscale items
  if (valuepos.size() > 256) {
    auto const inc = static_cast<float>(valuepos.size()) / 256.0f;
    {
      auto const keys = [valuepos=std::cref(valuepos)] ( ) {
				auto result = get_keys(valuepos.get( ));
      	std::sort(result.begin(), result.end());
				return result;
			}( );

      // TODO: make parallel
      for (size_t n = 0; n < keys.size(); ++n) {
        auto const cur_val = static_cast<int32_t>(static_cast<float>(n) / inc);
        auto const cur_key = static_cast<int32_t>(keys[n]);
        valuepos[cur_key] = cur_val;
      }
    }

    GenericImage<rgb3> image_output(image_input.width(), image_input.height());

    // TODO: make parallel
    daw::container::transform(
        image_input, image_output.begin(), [&vp = valuepos](rgb3 const &rgb) {
          return static_cast<uint8_t>(vp[FilterDAWGS::too_gs(rgb)]);
        });
    return image_output;
  } else { // Already a grayscale image or has enough room for all
           // possible values and no compression needed
    std::cerr << "Already a grayscale image or has enough room for all "
                 "possible values and no compression needed:"
              << valuepos.size() << std::endl;
    GenericImage<rgb3> image_output(image_input.width(), image_input.height());

    assert(image_input.size() <= image_output.size());
    // TODO: make parallel
    daw::container::transform(image_input, image_output.begin(),
                              [](rgb3 const &rgb) {
                                return static_cast<uint8_t>(rgb.too_float_gs());
                              });
    return image_output;
  }
}

int32_t FilterDAWGS::too_gs(rgb3 const &pixel) {
  return 19595 * pixel.red + 38469 * pixel.green +
         7471 * pixel.blue; // 0.299r + 0.587g + 0.114b
}

#ifdef DAWFILTER_USEPYTHON
void FilterDAWGS::register_python(std::string const nameoftype) {
  boost::python::def(nameoftype.c_str(), &FilterDAWGS::filter);
}
#endif
} // namespace imaging
} // namespace daw
