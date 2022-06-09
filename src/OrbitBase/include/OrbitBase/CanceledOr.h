// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_CANCELED_OR_H_
#define ORBIT_BASE_CANCELED_OR_H_

#include <variant>

#include "OrbitBase/VoidToMonostate.h"

namespace orbit_base {

struct Canceled {};

template <typename T>
using CanceledOr = std::variant<orbit_base_internal::VoidToMonostate_t<T>, Canceled>;

template <typename T>
[[nodiscard]] bool IsCanceled(const std::variant<T, Canceled>& canceled_or) {
  return std::holds_alternative<Canceled>(canceled_or);
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_CANCELED_OR_H_