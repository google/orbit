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

template <int dim>
void pick(GlSlider& slider, int start, int other_dim = 0);

template <>
void pick<0>(GlSlider& slider, int start, int other_dim) {
  slider.OnPick(start, other_dim);
}

template <>
void pick<1>(GlSlider& slider, int start, int other_dim) {
  slider.OnPick(other_dim, start);
}

template <int dim>
void drag(GlSlider& slider, int end, int other_dim = 0);

template <>
void drag<0>(GlSlider& slider, int end, int other_dim) {
  slider.OnDrag(end, other_dim);
}

template <>
void drag<1>(GlSlider& slider, int end, int other_dim) {
  slider.OnDrag(other_dim, end);
}

template <int dim>
void pick_drag(GlSlider& slider, int start, int end = -1, int other_dim = 0) {
  if (end < 0) {
    end = start;
  }
  pick<dim>(slider, start, other_dim);
  drag<dim>(slider, end, other_dim);
}

template <int dim>
void pick_drag_release(GlSlider& slider, int start, int end = -1, int other_dim = 0) {
  pick_drag<dim>(slider, start, end, other_dim);
  slider.OnRelease();
}

template <typename SliderClass>
std::pair<std::unique_ptr<SliderClass>, std::unique_ptr<MockCanvas>> setup() {
  std::unique_ptr<MockCanvas> canvas = std::make_unique<MockCanvas>();
  // Simulate a 1000x100 canvas
  EXPECT_CALL(*canvas, GetWidth()).WillRepeatedly(::testing::Return(150));
  EXPECT_CALL(*canvas, GetHeight()).WillRepeatedly(::testing::Return(1050));

  std::unique_ptr<SliderClass> slider = std::make_unique<SliderClass>();
  slider->SetCanvas(canvas.get());
  slider->SetPixelHeight(10);
  slider->SetOrthogonalSliderSize(50);

  // Set the slider to be 50% of the maximum size, position in the middle
  slider->SetSliderPosRatio(0.5f);
  slider->SetSliderLengthRatio(0.5f);

  return std::make_pair<std::unique_ptr<SliderClass>, std::unique_ptr<MockCanvas>>(
      std::move(slider), std::move(canvas));
}

const float kEpsilon = 0.01f;

template <typename SliderClass, int dim>
void test_drag_type() {
  auto [slider, canvas] = setup<SliderClass>();

  const float kPos = 0.5f;
  const float kSize = 0.5f;
  const float kOffset = 2;

  int drag_count = 0;
  float pos = kPos;
  int size_count = 0;
  float size = kSize;

  slider->SetDragCallback([&](float ratio) {
    ++drag_count;
    pos = ratio;
  });
  slider->SetResizeCallback([&](float ratio) {
    ++size_count;
    size = ratio;
  });

  // Use different scales for x and y to make sure dims are chosen correctly
  float scale = pow(10, dim);

  pick_drag_release<dim>(*slider, 50 * scale);
  EXPECT_EQ(drag_count, 1);
  EXPECT_EQ(size_count, 0);
  EXPECT_EQ(pos, kPos);
  EXPECT_EQ(size, kSize);

  pick_drag_release<dim>(*slider, 25 * scale + kOffset);
  EXPECT_EQ(drag_count, 2);
  EXPECT_EQ(size_count, 1);
  EXPECT_EQ(pos, kPos);
  EXPECT_EQ(size, kSize);

  pick_drag_release<dim>(*slider, 75 * scale - kOffset);
  EXPECT_EQ(drag_count, 3);
  EXPECT_EQ(size_count, 2);
  EXPECT_EQ(pos, kPos);
  EXPECT_EQ(size, kSize);

  pick_drag_release<dim>(*slider, kOffset);
  EXPECT_EQ(drag_count, 4);
  EXPECT_EQ(size_count, 2);
  EXPECT_NE(pos, kPos);
  EXPECT_EQ(size, kSize);

  pick_drag_release<dim>(*slider, 100 * scale - kOffset);
  EXPECT_EQ(drag_count, 5);
  EXPECT_EQ(size_count, 2);
  EXPECT_NE(pos, kPos);
  EXPECT_EQ(size, kSize);
}

TEST(Slider, DragType) {
  test_drag_type<GlHorizontalSlider, 0>();
  test_drag_type<GlVerticalSlider, 1>();
}

template <typename SliderClass, int dim>
void test_scroll(float slider_length = 0.25) {
  auto [slider, canvas] = setup<SliderClass>();

  // Use different scales for x and y to make sure dims are chosen correctly
  float scale = pow(10, dim);
  float pos;
  const float kOffset = 2;

  slider->SetDragCallback([&](float ratio) { pos = ratio; });
  slider->SetSliderLengthRatio(slider_length);

  pick_drag_release<dim>(*slider, kOffset);
  EXPECT_LT(pos, 0.5f);
  const float kCurPos = pos;

  pick_drag_release<dim>(*slider, 100 * scale - kOffset);
  EXPECT_GT(pos, kCurPos);
}

TEST(Slider, Scroll) {
  test_scroll<GlHorizontalSlider, 0>();
  test_scroll<GlVerticalSlider, 1>();
}

template <typename SliderClass, int dim>
void test_drag(float slider_length = 0.25) {
  auto [slider, canvas] = setup<SliderClass>();

  // Use different scales for x and y to make sure dims are chosen correctly
  float scale = pow(10, dim);
  float pos;

  slider->SetDragCallback([&](float ratio) { pos = ratio; });
  slider->SetSliderLengthRatio(slider_length);

  pick<dim>(*slider, 50 * scale);

  // Expect the slider to be dragged all the way to the right
  // (overshoot, first, then go back to exact drag pos)
  drag<dim>(*slider, 100 * scale);
  EXPECT_NEAR(pos, 1.f, kEpsilon);
  EXPECT_EQ(slider->GetPosRatio(), pos);
  drag<dim>(*slider, (100 - slider->GetLengthRatio() * 0.5f * 100) * scale);
  EXPECT_NEAR(pos, 1.f, kEpsilon);

  // Drag to middle
  drag<dim>(*slider, 50 * scale);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);

  // Drag to left
  drag<dim>(*slider, 0);
  EXPECT_NEAR(pos, 0.f, kEpsilon);

  // Back to middle
  drag<dim>(*slider, 50 * scale);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);

  // "Invalid" drags sanity check
  drag<dim>(*slider, 50 * scale, 5000);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);
}

TEST(Slider, Drag) {
  test_drag<GlHorizontalSlider, 0>();
  test_drag<GlVerticalSlider, 1>();
}

TEST(Slider, DragBreakTests) {
  test_drag<GlHorizontalSlider, 0>(0.0001f);
  test_drag<GlVerticalSlider, 1>(0.0001f);
}

template <typename SliderClass, int dim>
void test_scaling() {
  auto [slider, canvas] = setup<SliderClass>();

  float size = 0.5f;
  float pos = 0.5f;
  const float kOffset = 2;

  // Use different scales for x and y to make sure dims are chosen correctly
  float scale = pow(10, dim);

  slider->SetResizeCallback([&](float ratio) { size = ratio; });
  slider->SetDragCallback([&](float ratio) { pos = ratio; });

  // Pick on the left
  pick<dim>(*slider, 25 * scale + kOffset);

  // Resize 10% to the left, then all the way
  drag<dim>(*slider, 15 * scale + kOffset);
  EXPECT_NEAR(size, 0.6f, kEpsilon);
  EXPECT_NEAR(pos, 0.15f / 0.4f, kEpsilon);
  EXPECT_EQ(slider->GetLengthRatio(), size);
  EXPECT_EQ(slider->GetPosRatio(), pos);

  drag<dim>(*slider, 0);
  EXPECT_NEAR(size, 0.75f, kEpsilon);
  EXPECT_NEAR(pos, 0.f, kEpsilon);

  // Drag back
  drag<dim>(*slider, 25 * scale + kOffset);
  EXPECT_NEAR(size, 0.5f, kEpsilon);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);
  slider->OnRelease();

  // Pick on the right
  pick<dim>(*slider, 75 * scale - kOffset);

  // Resize 10% to the right, then all the way
  drag<dim>(*slider, 85 * scale - kOffset);
  EXPECT_NEAR(size, 0.6f, kEpsilon);
  EXPECT_NEAR(pos, 0.25f / 0.4f, kEpsilon);

  drag<dim>(*slider, 100 * scale);
  EXPECT_NEAR(size, 0.75f, kEpsilon);
  EXPECT_NEAR(pos, 1.f, kEpsilon);

  // Drag back
  drag<dim>(*slider, 75 * scale - kOffset);
  EXPECT_NEAR(size, 0.5f, kEpsilon);
  EXPECT_NEAR(pos, 0.25f / 0.5f, kEpsilon);
  slider->OnRelease();
}

template <typename SliderClass, int dim>
void test_break_scaling() {
  auto [slider, canvas] = setup<SliderClass>();

  float size;
  float pos;

  // Use different scales for x and y to make sure dims are chosen correctly
  float scale = pow(10, dim);

  // Position in the middle
  slider->SetSliderPosRatio(0.25f);
}

TEST(Slider, Scale) {
  test_scaling<GlHorizontalSlider, 0>();
  test_scaling<GlVerticalSlider, 1>();
}