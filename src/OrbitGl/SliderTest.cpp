// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>

#include <cmath>
#include <memory>
#include <tuple>
#include <utility>

#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CaptureViewElementTester.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/GlSlider.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

namespace {
template <typename T>
void CreateTestSlider(Viewport*, TimeGraphLayout*, std::unique_ptr<T>&) {}
template <>
void CreateTestSlider(Viewport* viewport, TimeGraphLayout* layout,
                      std::unique_ptr<GlHorizontalSlider>& result) {
  // The tests assume the default slider position and this specific width.
  result = std::make_unique<GlHorizontalSlider>(nullptr, viewport, layout, nullptr);
  result->SetWidth(viewport->GetScreenWidth() - layout->GetSliderWidth());
}
template <>
void CreateTestSlider(Viewport* viewport, TimeGraphLayout* layout,
                      std::unique_ptr<GlVerticalSlider>& result) {
  result = std::make_unique<GlVerticalSlider>(nullptr, viewport, layout, nullptr);
}

}  // namespace

template <int dim>
void Pick(GlSlider& slider, int start, int other_dim = 0) {
  static_assert(dim >= 0 && dim <= 1);
  if constexpr (dim == 0) {
    slider.OnPick(start, other_dim);
  } else {
    slider.OnPick(other_dim, start);
  }
}

template <int dim>
void Drag(GlSlider& slider, int end, int other_dim = 0) {
  static_assert(dim >= 0 && dim <= 1);
  if constexpr (dim == 0) {
    slider.OnDrag(end, other_dim);
  } else {
    slider.OnDrag(other_dim, end);
  }
}

template <int dim>
void PickDrag(GlSlider& slider, int start, int end = -1, int other_dim = 0) {
  static_assert(dim >= 0 && dim <= 1);
  if (end < 0) {
    end = start;
  }
  Pick<dim>(slider, start, other_dim);
  Drag<dim>(slider, end, other_dim);
}

template <int dim>
void PickDragRelease(GlSlider& slider, int start, int end = -1, int other_dim = 0) {
  static_assert(dim >= 0 && dim <= 1);
  PickDrag<dim>(slider, start, end, other_dim);
  slider.OnRelease();
}

template <typename SliderClass>
std::tuple<std::unique_ptr<SliderClass>, std::unique_ptr<Viewport>,
           std::unique_ptr<CaptureViewElementTester>>
Setup() {
  std::unique_ptr<CaptureViewElementTester> tester = std::make_unique<CaptureViewElementTester>();
  // Making space for the orthogonal slider as well.
  int orthogonal_slider_width = static_cast<int>(tester->GetLayout()->GetSliderWidth());
  std::unique_ptr<Viewport> viewport =
      std::make_unique<Viewport>(100 + orthogonal_slider_width, 1000 + orthogonal_slider_width);

  std::unique_ptr<SliderClass> slider;
  CreateTestSlider<SliderClass>(viewport.get(), tester->GetLayout(), slider);

  // Set the slider to be 50% of the maximum size, position in the middle
  slider->SetNormalizedPosition(0.5f);
  slider->SetNormalizedLength(0.5f);

  return std::make_tuple(std::move(slider), std::move(viewport), std::move(tester));
}

const float kEpsilon = 0.01f;

template <typename SliderClass, int dim>
static void TestDragType() {
  auto [slider, viewport, tester] = Setup<SliderClass>();

  const float kPos = 0.5f;
  const float kSize = 0.5f;
  const int kOffset = 2;

  int drag_count = 0;
  float pos = kPos;
  int size_count = 0;
  float size = kSize;

  slider->SetDragCallback([&](float ratio) {
    ++drag_count;
    pos = ratio;
  });
  slider->SetResizeCallback([&](float start, float end) {
    ++size_count;
    size = end - start;
  });

  // Use different scales for x and y to make sure dims are chosen correctly
  int scale = static_cast<int>(std::pow(10, dim));

  PickDragRelease<dim>(*slider, 50 * scale);
  EXPECT_EQ(drag_count, 1);
  EXPECT_EQ(size_count, 0);
  EXPECT_EQ(pos, kPos);
  EXPECT_EQ(size, kSize);

  PickDragRelease<dim>(*slider, 25 * scale + kOffset);
  if (slider->CanResize()) {
    EXPECT_EQ(drag_count, 2);
    EXPECT_EQ(size_count, 1);
  } else {
    EXPECT_EQ(drag_count, 2);
    EXPECT_EQ(size_count, 0);
  }
  EXPECT_NEAR(pos, kPos, kEpsilon);
  EXPECT_EQ(size, kSize);

  PickDragRelease<dim>(*slider, 75 * scale - kOffset);
  if (slider->CanResize()) {
    EXPECT_EQ(drag_count, 3);
    EXPECT_EQ(size_count, 2);
  } else {
    EXPECT_EQ(drag_count, 3);
    EXPECT_EQ(size_count, 0);
  }
  EXPECT_NEAR(pos, kPos, kEpsilon);
  EXPECT_EQ(size, kSize);

  drag_count = 0;
  size_count = 0;

  PickDragRelease<dim>(*slider, kOffset);
  EXPECT_EQ(drag_count, 1);
  EXPECT_EQ(size_count, 0);
  EXPECT_NE(pos, kPos);
  EXPECT_EQ(size, kSize);

  PickDragRelease<dim>(*slider, 100 * scale - kOffset);
  EXPECT_EQ(drag_count, 2);
  EXPECT_EQ(size_count, 0);
  EXPECT_NE(pos, kPos);
  EXPECT_EQ(size, kSize);
}

TEST(Slider, DragType) {
  TestDragType<GlHorizontalSlider, 0>();
  TestDragType<GlVerticalSlider, 1>();
}

template <typename SliderClass, int dim>
static void TestScroll(float slider_length = 0.25) {
  auto [slider, viewport, tester] = Setup<SliderClass>();

  // Use different scales for x and y to make sure dims are chosen correctly
  int scale = static_cast<int>(std::pow(10, dim));
  float pos;
  const int kOffset = 2;

  slider->SetDragCallback([&](float ratio) { pos = ratio; });
  slider->SetNormalizedLength(slider_length);

  PickDragRelease<dim>(*slider, kOffset);
  EXPECT_LT(pos, 0.5f);
  const float kCurPos = pos;

  PickDragRelease<dim>(*slider, 100 * scale - kOffset);
  EXPECT_GT(pos, kCurPos);
}

TEST(Slider, Scroll) {
  TestScroll<GlHorizontalSlider, 0>();
  TestScroll<GlVerticalSlider, 1>();
}

template <typename SliderClass, int dim>
static void TestDrag(float slider_length = 0.25) {
  auto [slider, viewport, tester] = Setup<SliderClass>();

  // Use different scales for x and y to make sure dims are chosen correctly
  int scale = static_cast<int>(std::pow(10, dim));
  float pos;

  slider->SetDragCallback([&](float ratio) { pos = ratio; });
  slider->SetNormalizedLength(slider_length);

  Pick<dim>(*slider, 50 * scale);

  // Expect the slider to be dragged all the way to the right
  // (overshoot, first, then go back to exact drag pos)
  Drag<dim>(*slider, 100 * scale);
  EXPECT_NEAR(pos, 1.f, kEpsilon);
  EXPECT_EQ(slider->GetPosRatio(), pos);
  Drag<dim>(*slider, 100 * scale - static_cast<int>(slider->GetSliderPixelLength() / 2));
  EXPECT_NEAR(pos, 1.f, kEpsilon);

  // Drag to middle
  Drag<dim>(*slider, 50 * scale);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);

  // Drag to left
  Drag<dim>(*slider, 0);
  EXPECT_NEAR(pos, 0.f, kEpsilon);

  // Back to middle
  Drag<dim>(*slider, 50 * scale);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);

  // "Invalid" drags sanity check
  Drag<dim>(*slider, 50 * scale, 5000);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);
}

TEST(Slider, Drag) {
  TestDrag<GlHorizontalSlider, 0>();
  TestDrag<GlVerticalSlider, 1>();
}

TEST(Slider, DragBreakTests) {
  TestDrag<GlHorizontalSlider, 0>(0.0001f);
  TestDrag<GlVerticalSlider, 1>(0.0001f);
}

template <typename SliderClass, int dim>
static void TestScaling() {
  auto [slider, viewport, tester] = Setup<SliderClass>();

  if (!slider->CanResize()) {
    return;
  }

  float size = 0.5f;
  float pos = 0.5f;
  const int kOffset = 2;

  // Use different scales for x and y to make sure dims are chosen correctly
  int scale = static_cast<int>(std::pow(10, dim));

  slider->SetResizeCallback([&](float start, float end) { size = end - start; });
  slider->SetDragCallback([&](float ratio) { pos = ratio; });

  // Pick on the left
  Pick<dim>(*slider, 25 * scale + kOffset);

  // Resize 10% to the left, then all the way
  Drag<dim>(*slider, 15 * scale + kOffset);
  EXPECT_NEAR(size, 0.6f, kEpsilon);
  EXPECT_NEAR(pos, 0.15f / 0.4f, kEpsilon);
  EXPECT_EQ(slider->GetLengthRatio(), size);
  EXPECT_EQ(slider->GetPosRatio(), pos);

  Drag<dim>(*slider, 0);
  EXPECT_NEAR(size, 0.75f, kEpsilon);
  EXPECT_NEAR(pos, 0.f, kEpsilon);

  // Drag back
  Drag<dim>(*slider, 25 * scale + kOffset);
  EXPECT_NEAR(size, 0.5f, kEpsilon);
  EXPECT_NEAR(pos, 0.5f, kEpsilon);
  slider->OnRelease();

  // Pick on the right
  Pick<dim>(*slider, 75 * scale - kOffset);

  // Resize 10% to the right, then all the way
  Drag<dim>(*slider, 85 * scale - kOffset);
  EXPECT_NEAR(size, 0.6f, kEpsilon);
  EXPECT_NEAR(pos, 0.25f / 0.4f, kEpsilon);

  Drag<dim>(*slider, 100 * scale);
  EXPECT_NEAR(size, 0.75f, kEpsilon);
  EXPECT_NEAR(pos, 1.f, kEpsilon);

  // Drag back
  Drag<dim>(*slider, 75 * scale - kOffset);
  EXPECT_NEAR(size, 0.5f, kEpsilon);
  EXPECT_NEAR(pos, 0.25f / 0.5f, kEpsilon);
  slider->OnRelease();
}

TEST(Slider, Scale) {
  TestScaling<GlHorizontalSlider, 0>();
  TestScaling<GlVerticalSlider, 1>();
}

template <typename SliderClass, int dim>
static void TestBreakScaling() {
  auto [slider, viewport, tester] = Setup<SliderClass>();

  if (!slider->CanResize()) {
    return;
  }

  float pos;
  float len;
  const int kOffset = 2;

  // Use different scales for x and y to make sure dims are chosen correctly
  int scale = static_cast<int>(std::pow(10, dim));

  // Pick on the right, then drag across the end of the slider
  pos = slider->GetSliderPixelPos();
  len = slider->GetSliderPixelLength();
  PickDragRelease<dim>(*slider, 75 * scale - kOffset, 0);
  EXPECT_NEAR(slider->GetSliderPixelPos(), pos, kEpsilon);
  EXPECT_NEAR(slider->GetSliderPixelLength(), slider->GetMinSliderPixelLength(), kEpsilon);

  slider->SetNormalizedPosition(0.5f);
  slider->SetNormalizedLength(0.5f);

  // Pick on the left, then drag across the end of the slider
  PickDragRelease<dim>(*slider, 25 * scale + kOffset, 100 * scale);
  EXPECT_NEAR(slider->GetSliderPixelPos(), pos + len - slider->GetMinSliderPixelLength(), kEpsilon);
  EXPECT_NEAR(slider->GetSliderPixelLength(), slider->GetMinSliderPixelLength(), kEpsilon);
}

TEST(Slider, BreakScale) {
  TestBreakScaling<GlHorizontalSlider, 0>();
  TestBreakScaling<GlVerticalSlider, 1>();
}

TEST(Slider, ContainsScreenSpacePoint) {
  CaptureViewElementTester tester;
  Viewport* viewport = tester.GetViewport();

  GlVerticalSlider slider(nullptr, viewport, tester.GetLayout(), nullptr);
  tester.SimulatePreRender(&slider);

  Vec2 slider_pos = slider.GetPos();
  Vec2 slider_size = slider.GetSize();
  Vec2 middle_point_slider{slider_pos[0] + slider_size[0] / 2.f,
                           slider_pos[1] + slider_size[1] / 2.f};
  Vec2 outside_point_slider{slider_pos[0] + slider_size[0] * 2.f,
                            slider_pos[1] + slider_size[1] * 2.f};
  EXPECT_TRUE(slider.ContainsScreenSpacePoint(static_cast<int>(middle_point_slider[0]),
                                              static_cast<int>(middle_point_slider[1])));
  EXPECT_FALSE(slider.ContainsScreenSpacePoint(static_cast<int>(outside_point_slider[0]),
                                               static_cast<int>(outside_point_slider[1])));
}

TEST(Slider, MouseMoveRequestRedraw) {
  CaptureViewElementTester tester;
  Viewport* viewport = tester.GetViewport();

  std::unique_ptr<GlHorizontalSlider> slider;
  CreateTestSlider<GlHorizontalSlider>(viewport, tester.GetLayout(), slider);
  tester.SimulatePreRender(slider.get());

  Vec2 kPosInSlider = slider->GetPos();

  // Any mouse move or mouse leave should trigger a redraw in the slider.
  EXPECT_EQ(slider->HandleMouseEvent(CaptureViewElement::MouseEvent{
                CaptureViewElement::MouseEventType::kMouseMove, kPosInSlider}),
            CaptureViewElement::EventResult::kIgnored);
  tester.SimulateDrawLoopAndCheckFlags(slider.get(), true, false);

  EXPECT_EQ(slider->HandleMouseEvent(CaptureViewElement::MouseEvent{
                CaptureViewElement::MouseEventType::kMouseMove, kPosInSlider}),
            CaptureViewElement::EventResult::kIgnored);
  tester.SimulateDrawLoopAndCheckFlags(slider.get(), true, false);

  EXPECT_EQ(slider->HandleMouseEvent(
                CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kMouseLeave}),
            CaptureViewElement::EventResult::kIgnored);
  tester.SimulateDrawLoopAndCheckFlags(slider.get(), true, false);

  EXPECT_EQ(slider->HandleMouseEvent(CaptureViewElement::MouseEvent{
                CaptureViewElement::MouseEventType::kMouseMove, kPosInSlider}),
            CaptureViewElement::EventResult::kIgnored);
  tester.SimulateDrawLoopAndCheckFlags(slider.get(), true, false);
}

}  // namespace orbit_gl