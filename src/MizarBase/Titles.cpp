// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarBase/Titles.h"

#include <QString>
#include <string>
#include <string_view>

#include "MizarBase/BaselineOrComparison.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_base {

using orbit_base::LiftAndApply;

static QString QStringFromStringView(std::string_view view) {
  return QString::fromStdString(std::string(view));
}

orbit_mizar_base::Baseline<QString> QBaselineTitle() {
  return LiftAndApply(&QStringFromStringView, kBaselineTitle);
}

orbit_mizar_base::Comparison<QString> QComparisonTitle() {
  return LiftAndApply(&QStringFromStringView, kComparisonTitle);
}

}  // namespace orbit_mizar_base