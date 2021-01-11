// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <array>
#include <limits>

#include "CodeViewer/Viewer.h"
#include "gtest/gtest.h"

namespace orbit_code_viewer {

static int argc = 0;

TEST(Viewer, DetermineLineNumberWidthInPixels) {
  QApplication app{argc, nullptr};

  const QFont font = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  const QFontMetrics font_metrics{font};

  const std::array max_line_numbers = {20, 99, 333, 999, 2000};

  for (const int max_line_number : max_line_numbers) {
    const int number_of_pixels_needed =
        DetermineLineNumberWidthInPixels(font_metrics, max_line_number);

    // All the line numbers between 1 and <max_line_number> need to fit
    // in the width calculated by DetermineLineNumberWidthInPixels.
    for (int line_number = 1; line_number <= max_line_number; ++line_number) {
      const auto line_number_label = QString::number(line_number);
      EXPECT_LE(font_metrics.horizontalAdvance(line_number_label), number_of_pixels_needed);
    }
  }

  (void)app;
}

}  // namespace orbit_code_viewer