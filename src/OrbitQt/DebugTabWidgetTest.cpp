// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QSignalSpy>
#include <QSlider>
#include <QTabWidget>
#include <string>

#include "OrbitGl/CaptureWindowDebugInterface.h"
#include "OrbitQt/CaptureWindowDebugWidget.h"
#include "OrbitQt/DebugTabWidget.h"

namespace {
class MockCaptureWindowDebugInterface : public orbit_gl::CaptureWindowDebugInterface {
 public:
  MOCK_METHOD(std::string, GetCaptureInfo, (), (const, override));
  MOCK_METHOD(std::string, GetPerformanceInfo, (), (const, override));
  MOCK_METHOD(std::string, GetSelectionSummary, (), (const, override));
};
}  // namespace

TEST(DebugTabWidget, NestedTabsEnabled) {
  MockCaptureWindowDebugInterface debug_interface{};
  EXPECT_CALL(debug_interface, GetCaptureInfo())
      .WillRepeatedly(testing::Return("This is the capture info."));
  EXPECT_CALL(debug_interface, GetPerformanceInfo())
      .WillRepeatedly(testing::Return("This is the performance info."));
  EXPECT_CALL(debug_interface, GetSelectionSummary())
      .WillRepeatedly(testing::Return("This is the selection summary."));

  orbit_qt::DebugTabWidget widget{};

  const auto* tab_widget = widget.findChild<QTabWidget*>("tabWidget");
  EXPECT_EQ(tab_widget->count(), 2);

  EXPECT_FALSE(tab_widget->isTabEnabled(0));
  EXPECT_FALSE(tab_widget->isTabEnabled(1));

  widget.SetCaptureWindowDebugInterface(&debug_interface);
  EXPECT_TRUE(tab_widget->isTabEnabled(0));
  EXPECT_FALSE(tab_widget->isTabEnabled(1));

  widget.SetIntrospectionWindowDebugInterface(&debug_interface);
  EXPECT_TRUE(tab_widget->isTabEnabled(0));
  EXPECT_TRUE(tab_widget->isTabEnabled(1));

  widget.ResetCaptureWindowDebugInterface();
  EXPECT_FALSE(tab_widget->isTabEnabled(0));
  EXPECT_TRUE(tab_widget->isTabEnabled(1));

  widget.ResetIntrospectionWindowDebugInterface();
  EXPECT_FALSE(tab_widget->isTabEnabled(0));
  EXPECT_FALSE(tab_widget->isTabEnabled(1));
}

TEST(DebugTabWidget, GetTimeGraphLayout) {
  orbit_qt::DebugTabWidget widget{};

  // The widget should offer a time graph layout for the capture window
  EXPECT_NE(widget.GetCaptureWindowTimeGraphLayout(), nullptr);

  // The widget should offer a time graph layout for the introspection window
  EXPECT_NE(widget.GetIntrospectionWindowTimeGraphLayout(), nullptr);

  // The debug tab widget has two signals which fire when any of the properties in the two time
  // graph layouts changes.
  QSignalSpy capture_window_spy{&widget,
                                &orbit_qt::DebugTabWidget::AnyCaptureWindowPropertyChanged};
  QSignalSpy introspection_window_spy{
      &widget, &orbit_qt::DebugTabWidget::AnyIntrospectionWindowPropertyChanged};

  // To change a property we need to make a change using one of the control sliders.
  const auto* capture_window_debug_widget =
      widget.findChild<orbit_qt::CaptureWindowDebugWidget*>("captureWindowDebugWidget");
  ASSERT_NE(capture_window_debug_widget, nullptr);

  auto* capture_window_text_box_height_slider =
      capture_window_debug_widget->findChild<QSlider*>("slider_text_box_height_");
  ASSERT_NE(capture_window_text_box_height_slider, nullptr);

  // We trigger a property change and check whether the correct signal has fired.
  capture_window_text_box_height_slider->setValue(100.f);
  EXPECT_EQ(capture_window_spy.size(), 1);
  EXPECT_EQ(introspection_window_spy.size(), 0);
  capture_window_spy.clear();
  introspection_window_spy.clear();

  const auto* introspection_window_debug_widget =
      widget.findChild<orbit_qt::CaptureWindowDebugWidget*>("introspectionWindowDebugWidget");
  ASSERT_NE(introspection_window_debug_widget, nullptr);

  auto* introspection_window_text_box_height_slider =
      introspection_window_debug_widget->findChild<QSlider*>("slider_text_box_height_");
  ASSERT_NE(introspection_window_text_box_height_slider, nullptr);

  // We trigger another property change and check whether the correct signal has fired.
  introspection_window_text_box_height_slider->setValue(100.f);
  EXPECT_EQ(capture_window_spy.size(), 0);
  EXPECT_EQ(introspection_window_spy.size(), 1);
}

// Use command line options `--gtest_filter=DebugTabWidget.DISABLED_Demo
// --gtest_also_run_disabled_tests` to run this demo.
TEST(DebugTabWidget, DISABLED_Demo) {
  MockCaptureWindowDebugInterface debug_interface{};
  EXPECT_CALL(debug_interface, GetCaptureInfo())
      .WillRepeatedly(testing::Return("This is the capture info."));
  EXPECT_CALL(debug_interface, GetPerformanceInfo())
      .WillRepeatedly(testing::Return("This is the performance info."));
  EXPECT_CALL(debug_interface, GetSelectionSummary())
      .WillRepeatedly(testing::Return("This is the selection summary."));

  orbit_qt::DebugTabWidget widget{};
  widget.SetCaptureWindowDebugInterface(&debug_interface);
  widget.SetIntrospectionWindowDebugInterface(&debug_interface);
  widget.show();

  QApplication::exec();
  SUCCEED();
}