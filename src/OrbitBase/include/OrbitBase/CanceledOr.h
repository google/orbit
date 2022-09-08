// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_CANCELED_OR_H_
#define ORBIT_BASE_CANCELED_OR_H_

#include <variant>

#include "OrbitBase/Logging.h"
#include "OrbitBase/VoidToMonostate.h"

namespace orbit_base {

// Type to indicate a CanceledOr type is canceled.
struct Canceled {};

// CanceledOr can be used as the return type of an cancelable operation. Based on std::variant.
// Check whether CanceledOr object is canceled, use orbit_base::IsCanceled. Get the value of a non
// canceled CanceledOr with std::get<T>().
template <typename T>
using CanceledOr = std::variant<VoidToMonostate_t<T>, Canceled>;

// Free function to quickly check whether a CanceledOr type is canceled. Abstracts
// std::holds_alternative
template <typename T>
[[nodiscard]] bool IsCanceled(const std::variant<T, Canceled>& canceled_or) {
  return std::holds_alternative<Canceled>(canceled_or);
}

// Free function to get the "Not canceled" content of a CanceledOr object. Abstracts std::get
template <typename T>
[[nodiscard]] const T& GetNotCanceled(const std::variant<T, Canceled>& canceled_or) {
  ORBIT_CHECK(!IsCanceled(canceled_or));
  return std::get<T>(canceled_or);
}

// Free function with move semantics to get the "Not canceled" content of a CanceledOr object.
// Abstracts std::get
template <typename T>
[[nodiscard]] T&& GetNotCanceled(std::variant<T, Canceled>&& canceled_or) {
  ORBIT_CHECK(!IsCanceled(canceled_or));
  return std::get<T>(std::move(canceled_or));
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_CANCELED_OR_H_