// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_TITLES_H_
#define MIZAR_BASE_TITLES_H_

#include <QString>
#include <string_view>

#include "MizarBase/BaselineOrComparison.h"

namespace orbit_mizar_base {

constexpr orbit_mizar_base::Baseline<std::string_view> kBaselineTitle("Baseline");
constexpr orbit_mizar_base::Comparison<std::string_view> kComparisonTitle("Comparison");

[[nodiscard]] orbit_mizar_base::Baseline<QString> QBaselineTitle();

[[nodiscard]] orbit_mizar_base::Comparison<QString> QComparisonTitle();

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_TITLES_H_
