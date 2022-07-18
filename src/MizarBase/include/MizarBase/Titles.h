// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_BASE_TITLES_H_
#define MIZAR_BASE_TITLES_H_

#include <QString>

#include "MizarBase/BaselineOrComparison.h"

namespace orbit_mizar_base {

static const orbit_mizar_base::Baseline<QString> kBaselineTitle =
    orbit_mizar_base::Baseline<QString>(QStringLiteral("Baseline"));
static const orbit_mizar_base::Comparison<QString> kComparisonTitle =
    orbit_mizar_base::Comparison<QString>(QStringLiteral("Comparison"));

}  // namespace orbit_mizar_base

#endif  // MIZAR_BASE_TITLES_H_
