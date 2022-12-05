// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <initializer_list>
#include <type_traits>
#include <vector>

#ifndef ORBIT_BASE_APPEND_H_
#define ORBIT_BASE_APPEND_H_

namespace orbit_base {

// Append adds the elements given by the range in the second argument to the back of the vector
// given in the first argument. The range can be anything that defines `Range::value_type` and has
// cbegin and cend iterators.
template <
    typename T, typename Range,
    typename = typename std::enable_if_t<std::is_convertible_v<typename Range::value_type, T>>>
void Append(std::vector<T>& dest, const Range& source) {
  using std::cbegin;
  using std::cend;
  dest.insert(std::end(dest), cbegin(source), cend(source));
}

// This overload makes initializer lists as the second argument work.
template <typename T, typename U, typename = typename std::enable_if_t<std::is_convertible_v<U, T>>>
void Append(std::vector<T>& dest, std::initializer_list<U> source) {
  using std::cbegin;
  using std::cend;
  dest.insert(std::end(dest), cbegin(source), cend(source));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_APPEND_H_
