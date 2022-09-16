// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_BASELINE_OR_COMPARISON_H_
#define MIZAR_BASE_BASELINE_OR_COMPARISON_H_

#include <utility>

#include "OrbitBase/Typedef.h"

namespace orbit_mizar_base {

struct BaselineTag {};
struct ComparisonTag {};

template <typename T>
using Baseline = orbit_base::Typedef<BaselineTag, T>;

template <typename T>
using Comparison = orbit_base::Typedef<ComparisonTag, T>;

template <typename T, typename... Args>
[[nodiscard]] Baseline<T> MakeBaseline(Args&&... args) {
  return Baseline<T>(T{std::forward<Args>(args)...});
}

template <typename T, typename... Args>
[[nodiscard]] Comparison<T> MakeComparison(Args&&... args) {
  return Comparison<T>(T{std::forward<Args>(args)...});
}

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_BASELINE_OR_COMPARISON_H_
