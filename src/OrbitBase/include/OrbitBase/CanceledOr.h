// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_CANCELED_OR_H_
#define ORBIT_BASE_CANCELED_OR_H_

#include <utility>
#include <variant>

#include "OrbitBase/Logging.h"
#include "OrbitBase/VoidToMonostate.h"

namespace orbit_base {

// Type to indicate a CanceledOr type is canceled.
struct Canceled {};

// CanceledOr can be used as the return type of an cancelable operation. Based on std::variant.
// Check whether CanceledOr object is canceled, use orbit_base::IsCanceled. Get the value of a non
// canceled CanceledOr with the `GetValue` member function or the `GetNotCanceled` free function.
//
// `CanceledOr<T>` is implemented by privately deriving from `std::variant`. This allows us to take
// advantage of std::variant's carefully crafted constructors while still keeping std::variant an
// implementation detail. Implementing those constructors by hand would add a lot of complexity.
template <typename T>
class CanceledOr : private std::variant<VoidToMonostate_t<T>, Canceled> {
 public:
  using Value = VoidToMonostate_t<T>;

  [[nodiscard]] bool HasValue() const { return this->index() == 0; }
  [[nodiscard]] bool IsCanceled() const { return !HasValue(); }

  [[nodiscard]] const Value& GetValue() const& { return std::get<0>(*this); }
  [[nodiscard]] const Value&& GetValue() const&& { return std::get<0>(*this); }
  [[nodiscard]] Value&& GetValue() && { return std::get<0>(std::move(*this)); }

  using std::variant<Value, Canceled>::variant;
  using std::variant<Value, Canceled>::operator=;
};

// Free function to quickly check whether a CanceledOr type is canceled.
template <typename T>
[[nodiscard]] bool IsCanceled(const CanceledOr<T>& canceled_or) {
  return canceled_or.IsCanceled();
}

// Free function to get the "Not canceled" content of a CanceledOr object.
template <typename T>
[[nodiscard]] const T& GetNotCanceled(const CanceledOr<T>& canceled_or) {
  ORBIT_CHECK(!IsCanceled(canceled_or));
  return canceled_or.GetValue();
}

// Free function with move semantics to get the "Not canceled" content of a CanceledOr object.
template <typename T>
[[nodiscard]] T&& GetNotCanceled(CanceledOr<T>&& canceled_or) {
  ORBIT_CHECK(!IsCanceled(canceled_or));
  return std::move(canceled_or).GetValue();
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_CANCELED_OR_H_