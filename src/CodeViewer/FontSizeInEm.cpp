// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <CodeViewer/FontSizeInEm.h>

#include <cmath>

namespace orbit_code_viewer {

int FontSizeInEm::ToPixels(const QFontMetrics& font_metrics) const {
  const int pixels_per_em = font_metrics.horizontalAdvance('M');
  return static_cast<int>(std::ceil(static_cast<float>(pixels_per_em) * value_));
}

}  // namespace orbit_code_viewer