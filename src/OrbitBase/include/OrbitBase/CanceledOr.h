// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_CANCELED_OR_H_
#define ORBIT_BASE_CANCELED_OR_H_

#include <string_view>
#include <utility>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_base {

// Type to indicate a CanceledOr type is canceled.
struct Canceled {
  [[nodiscard]] constexpr static std::string_view message() {
    return "The operation was canceled.";
  }
};

// CanceledOr can be used as the return type of an cancelable operation.
// Check whether CanceledOr object is canceled, use orbit_base::IsCanceled or `._has_error()`.
// Get the value of a non canceled CanceledOr with `.value()` or `GetNonCanceled(...);`
template <typename T>
using CanceledOr = Result<T, Canceled>;

// Free function to quickly check whether a CanceledOr type is canceled.
template <typename T>
[[nodiscard]] bool IsCanceled(const CanceledOr<T>& canceled_or) {
  return canceled_or.has_error();
}

// Free function to get the "Not canceled" content of a CanceledOr object.
template <typename T>
[[nodiscard]] const T& GetNotCanceled(const CanceledOr<T>& canceled_or) {
  ORBIT_CHECK(!IsCanceled(canceled_or));
  return canceled_or.value();
}

// Free function with move semantics to get the "Not canceled" content of a CanceledOr object.
template <typename T>
[[nodiscard]] T&& GetNotCanceled(CanceledOr<T>&& canceled_or) {
  ORBIT_CHECK(!IsCanceled(canceled_or));
  return std::move(canceled_or).value();
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_CANCELED_OR_H_