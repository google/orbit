// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <CodeViewer/FontSizeInEm.h>
#include <gtest/gtest.h>

#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <memory>

namespace orbit_code_viewer {

TEST(FontSizeInEm, NewObjectHasGivenValueZero) {
  FontSizeInEm null{};
  EXPECT_EQ(null.Value(), 0.0f);
}

TEST(FontSizeInEm, NewObjectHasGivenValueOne) {
  FontSizeInEm one{1.0f};
  EXPECT_EQ(one.Value(), 1.0f);
}

TEST(FontSizeInEm, Addition) {
  FontSizeInEm lhs{1.0f};
  FontSizeInEm rhs{2.2f};

  FontSizeInEm result{lhs + rhs};
  EXPECT_EQ(result.Value(), 1.0f + 2.2f);
}

TEST(FontSizeInEm, Subtraction) {
  FontSizeInEm lhs{1.0f};
  FontSizeInEm rhs{2.2f};

  FontSizeInEm result{lhs - rhs};
  EXPECT_EQ(result.Value(), 1.0f - 2.2f);
}

TEST(FontSizeInEm, ToPixels) {
  const QFont general_font = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
  const QFont fixed_font = QFontDatabase::systemFont(QFontDatabase::FixedFont);

  FontSizeInEm em1{1.2f};

  // There is not much invariance we can test. The only thing we know for sure
  // is that the width needs to be greater than 0px.
  EXPECT_GT(em1.ToPixels(QFontMetrics{general_font}), 0);
  EXPECT_GT(em1.ToPixels(QFontMetrics{fixed_font}), 0);
}

}  // namespace orbit_code_viewer