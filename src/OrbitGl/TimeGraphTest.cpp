// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>

#include <memory>
#include <vector>

#include "ClientData/CaptureData.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CaptureViewElementTester.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/GlSlider.h"
#include "OrbitGl/StaticTimeGraphLayout.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TimelineUi.h"
#include "OrbitGl/TrackTestData.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

class UnitTestTimeGraph : public testing::Test {
 public:
  explicit UnitTestTimeGraph() {
    capture_data_ = TrackTestData::GenerateTestCaptureData();
    // Make the viewport big enough to account for a left margin.
    viewport_ = std::make_unique<Viewport>(1000, 200);
    time_graph_ = std::make_unique<TimeGraph>(nullptr, nullptr, viewport_.get(),
                                              capture_data_.get(), nullptr, &time_graph_layout_);
    time_graph_->ZoomAll();
  }

  void SimulatePreRender() { tester_.SimulatePreRender(time_graph_.get()); }

  [[nodiscard]] TimeGraph* GetTimeGraph() const { return time_graph_.get(); }
  [[nodiscard]] GlSlider* HorizontalSlider() const { return time_graph_->GetHorizontalSlider(); }
  [[nodiscard]] GlSlider* VerticalSlider() const { return time_graph_->GetVerticalSlider(); }
  [[nodiscard]] TimelineUi* GetTimelineUi() const { return time_graph_->GetTimelineUi(); }

  void MouseMove(const Vec2& pos) {
    EXPECT_EQ(time_graph_->HandleMouseEvent(CaptureViewElement::MouseEvent{
                  CaptureViewElement::MouseEventType::kMouseMove, pos}),
              CaptureViewElement::EventResult::kIgnored);
  }
  void MouseLeave() {
    EXPECT_EQ(time_graph_->HandleMouseEvent(
                  CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kMouseLeave}),
              CaptureViewElement::EventResult::kIgnored);
  }
  void MouseWheelUp(const Vec2& pos, bool ctrl = false) {
    ModifierKeys modifier_keys;
    modifier_keys.ctrl = ctrl;
    EXPECT_EQ(
        time_graph_->HandleMouseEvent(
            CaptureViewElement::MouseEvent{CaptureViewElement::MouseEventType::kMouseWheelUp, pos},
            modifier_keys),
        CaptureViewElement::EventResult::kHandled);
  }

  void MouseWheelDown(const Vec2& pos, bool ctrl = false) {
    ModifierKeys modifier_keys;
    modifier_keys.ctrl = ctrl;
    EXPECT_EQ(time_graph_->HandleMouseEvent(
                  CaptureViewElement::MouseEvent{
                      CaptureViewElement::MouseEventType::kMouseWheelDown, pos},
                  modifier_keys),
              CaptureViewElement::EventResult::kHandled);
  }

  void CheckMouseIsOnlyOverAChild(CaptureViewElement* child_over_mouse) {
    EXPECT_TRUE(time_graph_->IsMouseOver());
    EXPECT_TRUE(child_over_mouse->IsMouseOver());
    for (auto child : GetTimeGraph()->GetNonHiddenChildren()) {
      if (child != child_over_mouse) {
        EXPECT_FALSE(child->IsMouseOver());
      }
    }
  }

 private:
  CaptureViewElementTester tester_;
  StaticTimeGraphLayout time_graph_layout_;
  std::unique_ptr<TimeGraph> time_graph_;
  std::unique_ptr<Viewport> viewport_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;
};

TEST_F(UnitTestTimeGraph, MouseMove) {
  SimulatePreRender();

  EXPECT_FALSE(GetTimeGraph()->IsMouseOver());

  MouseMove(HorizontalSlider()->GetPos());
  CheckMouseIsOnlyOverAChild(HorizontalSlider());

  MouseMove(VerticalSlider()->GetPos());
  CheckMouseIsOnlyOverAChild(VerticalSlider());

  MouseMove(GetTimelineUi()->GetPos());
  CheckMouseIsOnlyOverAChild(GetTimelineUi());
}

TEST_F(UnitTestTimeGraph, MouseLeave) {
  SimulatePreRender();

  MouseMove(HorizontalSlider()->GetPos());
  MouseLeave();
  EXPECT_FALSE(GetTimeGraph()->IsMouseOver());
  for (CaptureViewElement* child : GetTimeGraph()->GetNonHiddenChildren()) {
    EXPECT_FALSE(child->IsMouseOver());
  }
}

TEST_F(UnitTestTimeGraph, MouseWheel) {
  SimulatePreRender();
  const TimeGraph* time_graph = GetTimeGraph();

  double original_time_window_us = time_graph->GetTimeWindowUs();
  const orbit_gl::TimelineUi* timeline_ui = time_graph->GetTimelineUi();
  const Vec2 kLeftmostTimelineMousePosition = timeline_ui->GetPos();

  // The interactive area starts at the position of the timeline.
  const Vec2 kInteractiveAreaPos = {timeline_ui->GetPos()[0], timeline_ui->GetPos()[1]};
  // That area is the size of the timeline in X and extends to the size of the timegraph in Y.
  const Vec2 kInteractiveAreaSize = {timeline_ui->GetSize()[0], time_graph->GetSize()[1]};
  // Mouse position in the center of the interactive area.
  const Vec2 kCenteredMousePosition{kInteractiveAreaPos[0] + kInteractiveAreaSize[0] / 2.f,
                                    kInteractiveAreaPos[1] + kInteractiveAreaSize[1] / 2.f};

  MouseWheelUp(kCenteredMousePosition, /*ctrl=*/false);

  // MouseWheel should scroll when ctrl is not pressed and therefore not modify the time_window.
  // TODO(b/233855224): Test that vertical panning works as expected.
  EXPECT_EQ(original_time_window_us, time_graph->GetTimeWindowUs());

  MouseWheelUp(kCenteredMousePosition, /*ctrl=*/true);

  // With ctrl modifier, mouseWheel in the center should modify the time window.
  EXPECT_GT(original_time_window_us, time_graph->GetTimeWindowUs());

  // MouseWheelDown should revert the previous MouseWheelUp.
  MouseWheelDown(kCenteredMousePosition, /*ctrl=*/true);
  EXPECT_DOUBLE_EQ(original_time_window_us, time_graph->GetTimeWindowUs());

  // In the timeline, mouseWheel should also modify the time window. In addition, if the mouse is on
  // the leftmost position, the minimum visible timestamp shouldn't change.
  double min_visible_ns = time_graph->GetMinTimeUs();
  MouseWheelUp(kLeftmostTimelineMousePosition, /*ctrl=*/false);
  EXPECT_GT(original_time_window_us, time_graph->GetTimeWindowUs());
  EXPECT_EQ(min_visible_ns, time_graph->GetMinTimeUs());

  // Again, MouseWheelDown in the Timeline should revert the previous change.
  MouseWheelDown(kLeftmostTimelineMousePosition, /*ctrl=*/false);
  EXPECT_DOUBLE_EQ(original_time_window_us, time_graph->GetTimeWindowUs());
  EXPECT_EQ(min_visible_ns, time_graph->GetMinTimeUs());
}

}  // namespace orbit_gl