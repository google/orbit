// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "GlCanvas.h"
#include "GlSlider.h"

class MockCanvas : public GlCanvas {
 public:
  MOCK_METHOD(int, GetWidth, (), (const, override));
  MOCK_METHOD(int, GetHeight, (), (const, override));
};

TEST(Slider, Dragging) {
  using ::testing::Return;
  const float kEpsilon = 0.01f;

  MockCanvas canvas;
  // Simulate a 1000x100 canvas
  EXPECT_CALL(canvas, GetWidth()).WillRepeatedly(Return(1000));
  EXPECT_CALL(canvas, GetHeight()).WillRepeatedly(Return(100));

  float hpos;

  GlHorizontalSlider hslider;
  hslider.SetCanvas(&canvas);
  hslider.SetPixelHeight(10);

  // Set the slider to be 50% of the maximum size, position to the very left
  hslider.SetSliderPosRatio(0.f);
  hslider.SetSliderLengthRatio(0.5f);

  hslider.SetDragCallback([&](float ratio) { hpos = ratio; });

  hslider.OnPick(0, 0);
  hslider.OnDrag(1000, 0);

  // Expect the slider to be dragged all the way to the right
  // (overshoot, first, then go back to exact drag pos)
  EXPECT_NEAR(hpos, 1.f, kEpsilon);
  hslider.OnDrag(500, 0);
  EXPECT_NEAR(hpos, 1.0f, kEpsilon);

  // Drag to middle
  hslider.OnDrag(250, 0);
  EXPECT_NEAR(hpos, 0.5f, kEpsilon);

  // "Invalid" drags sanity check
  hslider.OnDrag(250, 100);
  EXPECT_NEAR(hpos, 0.5f, kEpsilon);

  hslider.OnRelease();

  // Tests when picked on the far right
  hslider.OnPick(500, 0);
  hslider.OnDrag(500, 0);
  EXPECT_NEAR(hpos, 0.f, kEpsilon);
}