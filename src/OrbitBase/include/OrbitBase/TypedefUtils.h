// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TYPEDEF_UTILS_H_
#define ORBIT_BASE_TYPEDEF_UTILS_H_

#include <type_traits>

namespace orbit_base_internal {

template <typename T, typename U>
using EnableIfUConvertibleToT = std::enable_if_t<std::is_convertible_v<U, T>>;

template <typename T, typename U>
using EnableIfUNotConvertibleToT = std::enable_if_t<!std::is_convertible_v<U, T>>;

}  // namespace orbit_base_internal

#endif  // ORBIT_BASE_TYPEDEF_UTILS_H_
