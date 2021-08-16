// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CaptureWindow.h"
#include "UnitTestSlider.h"

namespace orbit_gl {

class UnitTestCaptureWindow : public CaptureWindow, public testing::Test {
 public:
  explicit UnitTestCaptureWindow() : CaptureWindow(nullptr) {
    slider_ = std::make_unique<UnitTestHorizontalSlider>(viewport_);
    vertical_slider_ = std::make_unique<UnitTestVerticalSlider>(viewport_);
  }
};

TEST_F(UnitTestCaptureWindow, SlidersRespondToMouseOver) {
  Resize(100, 200);

  GlSlider* slider = FindSliderUnderMouseCursor(50, 50);
  EXPECT_EQ(nullptr, slider);
  slider = FindSliderUnderMouseCursor(95, 10);
  EXPECT_EQ(vertical_slider_.get(), slider);
  slider = FindSliderUnderMouseCursor(5, 195);
  EXPECT_EQ(slider_.get(), slider);
  slider = FindSliderUnderMouseCursor(95, 195);
  EXPECT_EQ(nullptr, slider);

  UnitTestHorizontalSlider* unit_test_slider =
      dynamic_cast<UnitTestHorizontalSlider*>(slider_.get());
  UnitTestVerticalSlider* unit_test_vertical_slider =
      dynamic_cast<UnitTestVerticalSlider*>(vertical_slider_.get());
  ;

  MouseMoved(95, 10, false, false, false);
  EXPECT_TRUE(unit_test_vertical_slider->IsMouseOver());

  MouseMoved(5, 195, false, false, false);
  EXPECT_FALSE(unit_test_vertical_slider->IsMouseOver());
  EXPECT_TRUE(unit_test_slider->IsMouseOver());

  MouseMoved(50, 50, false, false, false);
  EXPECT_FALSE(dynamic_cast<UnitTestVerticalSlider*>(vertical_slider_.get())->IsMouseOver());
  EXPECT_FALSE(unit_test_slider->IsMouseOver());
}

TEST_F(UnitTestCaptureWindow, SlidersBehaveCorrectlyWithMouseDown) {
  Resize(100, 200);
  MouseMoved(5, 195, true, false, false);

  UnitTestHorizontalSlider* unit_test_slider =
      dynamic_cast<UnitTestHorizontalSlider*>(slider_.get());
  UnitTestVerticalSlider* unit_test_vertical_slider =
      dynamic_cast<UnitTestVerticalSlider*>(vertical_slider_.get());

  EXPECT_FALSE(unit_test_vertical_slider->IsMouseOver());
  EXPECT_FALSE(unit_test_slider->IsMouseOver());

  MouseMoved(95, 10, false, false, false);
  LeftDown(95, 10);
  EXPECT_TRUE(unit_test_vertical_slider->IsMouseOver());
  MouseMoved(50, 50, true, false, false);
  EXPECT_TRUE(unit_test_vertical_slider->IsMouseOver());
  LeftUp();
  EXPECT_FALSE(unit_test_vertical_slider->IsMouseOver());
}

TEST_F(UnitTestCaptureWindow, SlidersRespondToMouseLeave) {
  Resize(100, 200);
  MouseMoved(95, 10, false, false, false);

  UnitTestVerticalSlider* unit_test_vertical_slider =
      dynamic_cast<UnitTestVerticalSlider*>(vertical_slider_.get());
  EXPECT_TRUE(unit_test_vertical_slider->IsMouseOver());
  SetIsMouseOver(false);
  EXPECT_FALSE(unit_test_vertical_slider->IsMouseOver());
}

}  // namespace orbit_gl