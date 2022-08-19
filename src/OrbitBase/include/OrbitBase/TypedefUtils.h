// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_TYPEDEF_UTILS_H_
#define ORBIT_BASE_TYPEDEF_UTILS_H_

#include <type_traits>
#include <utility>

namespace orbit_base_internal {

template <typename T, typename U>
using EnableIfUConvertibleToT = std::enable_if_t<std::is_convertible_v<U, T>>;

template <typename T, typename U>
using EnableIfUNotConvertibleToT = std::enable_if_t<!std::is_convertible_v<U, T>>;

template <typename OtherSummandTag>
struct PlusTagBase {};

struct DefaultPlus {
  template <typename T, typename U>
  constexpr auto operator()(T&& t, U&& u) const {
    return std::forward<T>(t) + std::forward<U>(u);
  }
};

constexpr DefaultPlus kDefaultPlus;

struct DefaultMinus {
  template <typename T, typename U>
  constexpr auto operator()(T&& t, U&& u) const {
    return std::forward<T>(t) - std::forward<U>(u);
  }
};

constexpr DefaultMinus kDefaultMinus;

struct DefaultTimes {
  template <typename T, typename U>
  constexpr auto operator()(T&& t, U&& u) const {
    return std::forward<T>(t) * std::forward<U>(u);
  }
};

constexpr DefaultTimes kDefaultTimes;

template <typename Scalar>
struct TimesScalarTagBase {};

}  // namespace orbit_base_internal

#endif  // ORBIT_BASE_TYPEDEF_UTILS_H_
