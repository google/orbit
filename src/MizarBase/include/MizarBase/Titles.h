// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_TITLES_H_
#define MIZAR_BASE_TITLES_H_

#include <QString>

#include "MizarBase/BaselineOrComparison.h"

namespace orbit_mizar_base {

[[nodiscard]] inline const orbit_mizar_base::Baseline<QString>& BaselineTitle() {
  static orbit_mizar_base::Baseline<QString> kTitle(QStringLiteral("Baseline"));
  return kTitle;
}
[[nodiscard]] inline const orbit_mizar_base::Comparison<QString>& ComparisonTitle() {
  static orbit_mizar_base::Comparison<QString> kTitle(QStringLiteral("Comparison"));
  return kTitle;
}

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_TITLES_H_
