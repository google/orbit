// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <QApplication>
#include <memory>

#include "OrbitBase/ThreadConstants.h"
#include "OrbitQt/TimeGraphLayoutWidget.h"

TEST(TimeGraphLayoutWidgetTest, SetScale) {
  orbit_qt::TimeGraphLayoutWidget widget{};
  // There is not a lot of logic in the TimeGraphLayoutWidget, but we can at least test the SetScale
  // function.

  const float previous_text_box_height = widget.GetTextBoxHeight();

  constexpr uint32_t kRandomThreadId = 1;
  const float previous_thread_track_height = widget.GetEventTrackHeightFromTid(kRandomThreadId);
  const float previous_all_threads_track_height =
      widget.GetEventTrackHeightFromTid(orbit_base::kAllProcessThreadsTid);

  // If the scaling doubles the text box height should double too.
  constexpr float kScaleFactor = 2.0f;
  widget.SetScale(kScaleFactor * widget.GetScale());
  EXPECT_FLOAT_EQ(kScaleFactor * previous_text_box_height, widget.GetTextBoxHeight());
  EXPECT_FLOAT_EQ(kScaleFactor * previous_thread_track_height,
                  widget.GetEventTrackHeightFromTid(kRandomThreadId));
  EXPECT_FLOAT_EQ(kScaleFactor * previous_all_threads_track_height,
                  widget.GetEventTrackHeightFromTid(orbit_base::kAllProcessThreadsTid));
}

// Start the test binary with `--gtest_filter=TimeGraphLayoutWidgetTest.DISABLED_Demo
// --gtest_also_run_disabled_tests` to run this demo.
TEST(TimeGraphLayoutWidgetTest, DISABLED_Demo) {
  orbit_qt::TimeGraphLayoutWidget widget{};
  widget.show();
  QApplication::exec();
  SUCCEED();
}