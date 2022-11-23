// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QPlainTextEdit>
#include <QString>
#include <QTest>
#include <string>

#include "OrbitGl/CaptureWindowDebugInterface.h"
#include "OrbitQt/CaptureWindowDebugWidget.h"

namespace {
class MockCaptureWindowDebugInterface : public orbit_gl::CaptureWindowDebugInterface {
 public:
  MOCK_METHOD(std::string, GetCaptureInfo, (), (const, override));
  MOCK_METHOD(std::string, GetPerformanceInfo, (), (const, override));
  MOCK_METHOD(std::string, GetSelectionSummary, (), (const, override));
};
}  // namespace

TEST(CaptureWindowDebugWidget, Simple) {
  MockCaptureWindowDebugInterface debug_interface{};
  EXPECT_CALL(debug_interface, GetCaptureInfo())
      .WillRepeatedly(testing::Return("This is the capture info."));
  EXPECT_CALL(debug_interface, GetPerformanceInfo())
      .WillRepeatedly(testing::Return("This is the performance info."));
  EXPECT_CALL(debug_interface, GetSelectionSummary())
      .WillRepeatedly(testing::Return("This is the selection summary."));

  orbit_qt::CaptureWindowDebugWidget widget{};
  widget.SetCaptureWindowDebugInterface(&debug_interface);

  const auto* capture_info_text_edit = widget.findChild<QPlainTextEdit*>("captureInfoTextEdit");
  ASSERT_NE(capture_info_text_edit, nullptr);
  EXPECT_THAT(capture_info_text_edit->toPlainText().toStdString(),
              testing::HasSubstr("capture info"));

  const auto* performance_info_text_edit = widget.findChild<QPlainTextEdit*>("performanceTextEdit");
  ASSERT_NE(performance_info_text_edit, nullptr);
  EXPECT_THAT(performance_info_text_edit->toPlainText().toStdString(),
              testing::HasSubstr("performance info"));

  const auto* selection_summary_text_edit =
      widget.findChild<QPlainTextEdit*>("selectionSummaryTextEdit");
  ASSERT_NE(selection_summary_text_edit, nullptr);
  EXPECT_THAT(selection_summary_text_edit->toPlainText().toStdString(),
              testing::HasSubstr("selection summary"));
}

TEST(CaptureWindowDebugWidget, DynamicUpdates) {
  MockCaptureWindowDebugInterface debug_interface{};
  EXPECT_CALL(debug_interface, GetCaptureInfo()).WillRepeatedly(testing::Return(""));
  EXPECT_CALL(debug_interface, GetPerformanceInfo()).WillRepeatedly(testing::Return(""));
  EXPECT_CALL(debug_interface, GetSelectionSummary()).WillRepeatedly(testing::Return(""));

  orbit_qt::CaptureWindowDebugWidget widget{};
  widget.SetCaptureWindowDebugInterface(&debug_interface);

  const auto* capture_info_text_edit = widget.findChild<QPlainTextEdit*>("captureInfoTextEdit");
  ASSERT_NE(capture_info_text_edit, nullptr);
  EXPECT_TRUE(capture_info_text_edit->toPlainText().isEmpty());

  const auto* performance_info_text_edit = widget.findChild<QPlainTextEdit*>("performanceTextEdit");
  ASSERT_NE(performance_info_text_edit, nullptr);
  EXPECT_TRUE(performance_info_text_edit->toPlainText().isEmpty());

  const auto* selection_summary_text_edit =
      widget.findChild<QPlainTextEdit*>("selectionSummaryTextEdit");
  ASSERT_NE(selection_summary_text_edit, nullptr);
  EXPECT_TRUE(selection_summary_text_edit->toPlainText().isEmpty());

  // After the first initial check, we change the returned results of the debug interface and expect
  // the widget update after some time.
  EXPECT_CALL(debug_interface, GetCaptureInfo())
      .WillRepeatedly(testing::Return("This is the capture info."));
  EXPECT_CALL(debug_interface, GetPerformanceInfo())
      .WillRepeatedly(testing::Return("This is the performance info."));
  EXPECT_CALL(debug_interface, GetSelectionSummary())
      .WillRepeatedly(testing::Return("This is the selection summary."));

  // The widget guarantees to update every 200ms (roughly), so we check after 500ms to avoid any
  // timing problems. Note that qWait runs an event loop in the background, it's not a sleep!
  QTest::qWait(500);

  EXPECT_THAT(capture_info_text_edit->toPlainText().toStdString(),
              testing::HasSubstr("capture info"));
  EXPECT_THAT(performance_info_text_edit->toPlainText().toStdString(),
              testing::HasSubstr("performance info"));
  EXPECT_THAT(selection_summary_text_edit->toPlainText().toStdString(),
              testing::HasSubstr("selection summary"));
}

// Use command line options `--gtest_filter=CaptureWindowDebugWidget.DISABLED_Demo
// --gtest_also_run_disabled_tests` to run this demo.
TEST(CaptureWindowDebugWidget, DISABLED_Demo) {
  MockCaptureWindowDebugInterface debug_interface{};
  EXPECT_CALL(debug_interface, GetCaptureInfo())
      .WillRepeatedly(testing::Return("This is the capture info."));
  EXPECT_CALL(debug_interface, GetPerformanceInfo())
      .WillRepeatedly(testing::Return("This is the performance info."));
  EXPECT_CALL(debug_interface, GetSelectionSummary())
      .WillRepeatedly(testing::Return("This is the selection summary."));

  orbit_qt::CaptureWindowDebugWidget widget{};
  widget.SetCaptureWindowDebugInterface(&debug_interface);
  widget.show();

  QApplication::exec();
  SUCCEED();
}