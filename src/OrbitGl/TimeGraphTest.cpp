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
  }

  // TODO(b/230441102): Temporal solution while we implement mouse moved methods for
  // CaptureViewElements.
  void MouseMoved(int x, int y, bool left, bool right, bool middle) {
    if (!(left || right || middle)) {
      time_graph_->ProcessSliderMouseMoveEvents(x, y);
    }
  }

  void SimulatePreRender() { tester_.SimulatePreRender(time_graph_.get()); }

  orbit_gl::GlSlider* FindSliderUnderMouseCursor(int x, int y) {
    return time_graph_->FindSliderUnderMouseCursor(x, y);
  }
  [[nodiscard]] GlSlider* Slider() { return time_graph_->GetHorizontalSlider(); }
  [[nodiscard]] GlSlider* VerticalSlider() { return time_graph_->GetVerticalSlider(); }

 private:
  CaptureViewElementTester tester_;
  std::unique_ptr<TimeGraph> time_graph_;
  std::unique_ptr<Viewport> viewport_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;
};

TEST_F(UnitTestTimeGraph, SlidersRespondToMouseOver) {
  SimulatePreRender();

  Vec2 kMouseCenteredPos{50, 100};
  GlSlider* slider = FindSliderUnderMouseCursor(static_cast<int>(kMouseCenteredPos[0]),
                                                static_cast<int>(kMouseCenteredPos[1]));
  EXPECT_EQ(nullptr, slider);
  EXPECT_FALSE(VerticalSlider()->IsMouseOver(kMouseCenteredPos));
  EXPECT_FALSE(Slider()->IsMouseOver(kMouseCenteredPos));

  Vec2 kMouseRightPos{95, 100};
  slider = FindSliderUnderMouseCursor(static_cast<int>(kMouseRightPos[0]),
                                      static_cast<int>(kMouseRightPos[1]));
  EXPECT_EQ(VerticalSlider(), slider);
  EXPECT_TRUE(VerticalSlider()->IsMouseOver(kMouseRightPos));
  EXPECT_FALSE(Slider()->IsMouseOver(kMouseRightPos));

  Vec2 kMouseBottomLeftPos{5, 195};
  slider = FindSliderUnderMouseCursor(static_cast<int>(kMouseBottomLeftPos[0]),
                                      static_cast<int>(kMouseBottomLeftPos[1]));
  EXPECT_EQ(Slider(), slider);
  EXPECT_FALSE(VerticalSlider()->IsMouseOver(kMouseBottomLeftPos));
  EXPECT_TRUE(Slider()->IsMouseOver(kMouseBottomLeftPos));
}

}  // namespace orbit_gl