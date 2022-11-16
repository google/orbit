// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>

#include "TimeGraphLayoutWidget.h"

TEST(TimeGraphLayoutWidgetTest, SetScale) {
  orbit_qt::TimeGraphLayoutWidget widget{};
  // There is not a lot of logic in the TimeGraphLayoutWidget, but we can at least test the SetScale
  // function.

  const float previous_text_box_height = widget.GetTextBoxHeight();

  // If the scaling doubles the text box height should double too.
  widget.SetScale(2.0f * widget.GetScale());
  EXPECT_FLOAT_EQ(2.0f * previous_text_box_height, widget.GetTextBoxHeight());
}

// Start the test binary with `--gtest_filter=TimeGraphLayoutWidgetTest.DISABLED_Demo
// --gtest_also_run_disabled_tests` to run this demo.
TEST(TimeGraphLayoutWidgetTest, DISABLED_Demo) {
  orbit_qt::TimeGraphLayoutWidget widget{};
  widget.show();
  QApplication::exec();
  SUCCEED();
}