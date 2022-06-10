// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_VOID_TO_MONOSTATE_H_
#define ORBIT_BASE_VOID_TO_MONOSTATE_H_

#include <cstddef>
#include <variant>

namespace orbit_base {

// A small helper type trait that maps `void` to `std::monostate` and every other `T` to itself.
template <typename T>
struct VoidToMonostate {
  using type = T;
};

template <>
struct VoidToMonostate<void> {
  using type = std::monostate;
};

template <typename T>
using VoidToMonostate_t = typename VoidToMonostate<T>::type;

// A small helper type trait that checks whether a certain index of a parameter pack `Args` is of
// type `std::monostate`. If yes the resulting type will be `std::true_type`, otherwise
// `std::false_type`.
template <size_t index, typename... Args>
using IsMonostate =
    std::is_same<std::variant_alternative_t<index, std::variant<VoidToMonostate_t<Args>...>>,
                 std::monostate>;

}  // namespace orbit_base

#endif  // ORBIT_BASE_VOID_TO_MONOSTATE_H_