// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "CaptureViewElementTester.h"
#include "TimeGraph.h"
#include "TrackTestData.h"
#include "Viewport.h"

namespace orbit_gl {

class UnitTestTimeGraph : public testing::Test {
 public:
  explicit UnitTestTimeGraph() {
    capture_data_ = TrackTestData::GenerateTestCaptureData();
    viewport_ = std::make_unique<Viewport>(100, 200);
    time_graph_ = std::make_unique<TimeGraph>(nullptr, nullptr, viewport_.get(),
                                              capture_data_.get(), nullptr);
    time_graph_->ZoomAll();
  }

  void SimulatePreRender() { tester_.SimulatePreRender(time_graph_.get()); }

  const TimeGraph* GetTimeGraph() { return time_graph_.get(); }
  [[nodiscard]] GlSlider* HorizontalSlider() { return time_graph_->GetHorizontalSlider(); }
  [[nodiscard]] GlSlider* VerticalSlider() { return time_graph_->GetVerticalSlider(); }
  [[nodiscard]] TimelineUi* TimelineUi() { return time_graph_->GetTimelineUi(); }

  void MouseMove(const Vec2& pos) {
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{CaptureViewElement::EventType::kMouseMove, pos});
  }
  void MouseLeave() {
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{CaptureViewElement::EventType::kMouseLeave});
  }
  void MouseWheelUp(const Vec2& pos) {
    std::ignore = time_graph_->HandleMouseEvent(
        CaptureViewElement::MouseEvent{CaptureViewElement::EventType::kMouseWheelUp, pos});
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

  MouseMove(TimelineUi()->GetPos());
  CheckMouseIsOnlyOverAChild(TimelineUi());
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

  double time_window_us = time_graph->GetTimeWindowUs();
  Vec2 kCenteredMousePosition{time_graph->GetPos()[0] + time_graph->GetSize()[0] / 2.f,
                              time_graph->GetPos()[1] + time_graph->GetSize()[1] / 2.f};
  MouseWheelUp(kCenteredMousePosition);

  // MouseWheel in the center should modify the time window. This will change soon.
  EXPECT_NE(time_window_us, time_graph->GetTimeWindowUs());

  time_window_us = time_graph->GetTimeWindowUs();
  MouseWheelUp(time_graph->GetTimelineUi()->GetPos());

  // MouseWheel in the timeline should modify the time windows.
  EXPECT_NE(time_window_us, time_graph->GetTimeWindowUs());
}

}  // namespace orbit_gl