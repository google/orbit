// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GteVector.h>
#include <gtest/gtest.h>

#include <memory>
#include <random>
#include <vector>

#include "CaptureClient/AppInterface.h"
#include "CaptureClient/CaptureClient.h"
#include "ClientData/CaptureData.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/CaptureWindow.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/GlSlider.h"
#include "OrbitGl/StaticTimeGraphLayout.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TimelineUi.h"
#include "OrbitGl/TrackContainer.h"
#include "OrbitGl/TrackTestData.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

class CaptureClientAppInterfaceFake : public orbit_capture_client::CaptureControlInterface {
  [[nodiscard]] orbit_capture_client::CaptureClient::State GetCaptureState() const {
    return orbit_capture_client::CaptureClient::State::kStopped;
  }

  [[nodiscard]] bool IsCapturing() const { return false; }

  void StartCapture() {}
  void StopCapture() {}
  void AbortCapture() {}
  void ToggleCapture() {}
};

constexpr int kBottomSafetyMargin = 5;
constexpr int kViewportWidth = 600;
constexpr int kViewportHeight = 100;

constexpr double kSliderPosEpsilon = 0.0001;
constexpr double kTimeEpsilonUs = 0.0000001;

class NavigationTestCaptureWindow : public testing::Test {
 public:
  explicit NavigationTestCaptureWindow()
      : capture_window_{nullptr, &capture_client_app_fake_, &time_graph_layout_} {
    capture_window_.Resize(kViewportWidth, kViewportHeight);

    capture_data_ = TrackTestData::GenerateTestCaptureData();
    capture_window_.CreateTimeGraph(capture_data_.get());

    AddTimers();
    capture_window_.PreRender();
    capture_window_.GetTimeGraph()->ZoomAll();

    // Make sure our expectations about the height of the timeline are correct,
    // otherwise zooming may not work as expected - if this fails, the overall
    // viewport height should be changed and the test adjusted accordingly
    ORBIT_CHECK(capture_window_.GetTimeGraph()->GetTimelineUi()->GetHeight() <
                kViewportHeight - kBottomSafetyMargin);
    ORBIT_CHECK(capture_window_.GetTimeGraph()->GetTimelineUi()->GetPos()[0] ==
                time_graph_layout_.GetLeftMargin());
  }

 protected:
  void ExpectInitialState(bool allow_small_imprecision = false) {
    EXPECT_LT(capture_window_.GetTimeGraph()->GetVerticalSlider()->GetLengthRatio(), 1.0);
    EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetVerticalSlider()->GetPosRatio(), 0.0);

    EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetLengthRatio(), 1.0);
    EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);

    if (allow_small_imprecision) {
      EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMaxTimeUs(),
                  capture_window_.GetTimeGraph()->GetCaptureMax() / 1000, kTimeEpsilonUs);
      EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMinTimeUs(), 0, kTimeEpsilonUs);
    } else {
      EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetMaxTimeUs() * 1000,
                       capture_window_.GetTimeGraph()->GetCaptureMax());
      EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetMinTimeUs() * 1000, 0);
    }
  }

  void ExpectScrolledDownFromInitialState() {
    EXPECT_GT(capture_window_.GetTimeGraph()->GetVerticalSlider()->GetPosRatio(), 0.0);

    EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetLengthRatio(), 1.0);
    EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);
  }

  enum struct PosWithinCapture { kLeft, kRight, kMiddle, kAnywhere };
  void ExpectIsHorizontallyZoomedIn(PosWithinCapture pos) {
    EXPECT_LT(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetLengthRatio(), 1.0);
    EXPECT_LT(capture_window_.GetTimeGraph()->GetMaxTimeUs() -
                  capture_window_.GetTimeGraph()->GetMinTimeUs(),
              capture_window_.GetTimeGraph()->GetCaptureTimeSpanUs());

    switch (pos) {
      case PosWithinCapture::kLeft:
        EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);
        EXPECT_LT(capture_window_.GetTimeGraph()->GetMaxTimeUs() * 1000,
                  capture_window_.GetTimeGraph()->GetCaptureMax());
        EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetMinTimeUs() * 1000, 0);
        break;
      case PosWithinCapture::kRight:
        // TODO (b/226376252): This should also check if pos + size == 100%
        EXPECT_GT(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);
        EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetMaxTimeUs() * 1000,
                         capture_window_.GetTimeGraph()->GetCaptureMax());
        EXPECT_GT(capture_window_.GetTimeGraph()->GetMinTimeUs() * 1000, 0);
        break;
      case PosWithinCapture::kMiddle:
        EXPECT_NEAR(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.5,
                    0.01);
        EXPECT_NEAR(capture_window_.GetTimeGraph()->GetCaptureMax() -
                        capture_window_.GetTimeGraph()->GetMaxTimeUs() * 1000,
                    capture_window_.GetTimeGraph()->GetMinTimeUs() * 1000 -
                        capture_window_.GetTimeGraph()->GetCaptureMin(),
                    1);
        break;
      case PosWithinCapture::kAnywhere:
        // Covered by basic conditions above
        break;
    }
  }

 private:
  CaptureClientAppInterfaceFake capture_client_app_fake_;
  StaticTimeGraphLayout time_graph_layout_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;

 protected:
  CaptureWindow capture_window_;

 private:
  void AddTimers() {
    auto timers = TrackTestData::GenerateTimers();
    for (auto& timer : timers) {
      capture_window_.GetTimeGraph()->ProcessTimer(timer);
    }
  }
};

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksInTheMiddleOfTimeline) {
  capture_window_.PreRender();
  ExpectInitialState();
  TimelineUi* timeline_ui = capture_window_.GetTimeGraph()->GetTimelineUi();
  const Vec2i kTimelineSize = capture_window_.GetViewport().WorldToScreen(timeline_ui->GetSize());
  const Vec2i kTimelinePos = capture_window_.GetViewport().WorldToScreen(timeline_ui->GetPos());

  int x = kTimelinePos[0] + kTimelineSize[0] / 2;
  int y = kTimelinePos[1] + kTimelineSize[1] / 2;

  capture_window_.MouseWheelMoved(x, y, 1, /*ctrl=*/false);
  capture_window_.PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kMiddle);

  capture_window_.MouseWheelMoved(x, y, -1, /*ctrl=*/false);
  capture_window_.PreRender();
  ExpectInitialState();

  // Keyboard
  capture_window_.MouseMoved(x, y, false, false, false);
  capture_window_.KeyPressed('W', false, false, false);
  capture_window_.PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kMiddle);

  capture_window_.KeyPressed('S', false, false, false);
  capture_window_.PreRender();
  ExpectInitialState();
}

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksAtRandomPositions) {
  capture_window_.PreRender();
  ExpectInitialState();
  const Vec2i kTimelineSize = capture_window_.GetViewport().WorldToScreen(
      capture_window_.GetTimeGraph()->GetTimelineUi()->GetSize());

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, kTimelineSize[0]);

  constexpr int kNumTries = 100;
  for (int i = 0; i < kNumTries; ++i) {
    const int x = dist(mt);
    const int y = kTimelineSize[1] - kBottomSafetyMargin;

    // Zoom in twice, then zoom out twice. Check that the intermediate states match
    capture_window_.MouseWheelMoved(x, y, 1, /*ctrl=*/true);
    capture_window_.PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    float last_slider_pos = capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio();
    double last_min_time = capture_window_.GetTimeGraph()->GetMinTimeUs();
    double last_max_time = capture_window_.GetTimeGraph()->GetMaxTimeUs();
    capture_window_.MouseWheelMoved(x, y, 1, /*ctrl=*/true);
    capture_window_.PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    capture_window_.MouseWheelMoved(x, y, -1, /*ctrl=*/true);
    capture_window_.PreRender();
    EXPECT_NEAR(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(),
                last_slider_pos, kSliderPosEpsilon);
    EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMinTimeUs(), last_min_time, kTimeEpsilonUs);
    EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMaxTimeUs(), last_max_time, kTimeEpsilonUs);

    capture_window_.MouseWheelMoved(x, y, -1, /*ctrl=*/true);
    capture_window_.PreRender();
    ExpectInitialState(true);

    // Same test using the Keyboard
    capture_window_.MouseMoved(x, y, false, false, false);
    capture_window_.KeyPressed('W', false, false, false);
    capture_window_.PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    last_slider_pos = capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio();
    last_min_time = capture_window_.GetTimeGraph()->GetMinTimeUs();
    last_max_time = capture_window_.GetTimeGraph()->GetMaxTimeUs();
    capture_window_.KeyPressed('W', false, false, false);
    capture_window_.PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    capture_window_.KeyPressed('S', false, false, false);
    capture_window_.PreRender();
    EXPECT_NEAR(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(),
                last_slider_pos, kSliderPosEpsilon);
    EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMinTimeUs(), last_min_time, kTimeEpsilonUs);
    EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMaxTimeUs(), last_max_time, kTimeEpsilonUs);

    capture_window_.KeyPressed('S', false, false, false);
    capture_window_.PreRender();
    ExpectInitialState(true);
  }
}

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksAtTheRightOfTimeline) {
  capture_window_.PreRender();
  ExpectInitialState();

  TimelineUi* timeline_ui = capture_window_.GetTimeGraph()->GetTimelineUi();
  const Vec2i kTimelineSize = capture_window_.GetViewport().WorldToScreen(timeline_ui->GetSize());
  const Vec2i kTimelinePos = capture_window_.GetViewport().WorldToScreen(timeline_ui->GetPos());

  int x = kTimelinePos[0] + kTimelineSize[0];
  int y = kTimelinePos[1] + kTimelineSize[1] / 2;

  capture_window_.MouseWheelMoved(x, y, 1, /*ctrl=*/true);
  capture_window_.PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kRight);

  capture_window_.MouseWheelMoved(x, y, -1, /*ctrl=*/true);
  capture_window_.PreRender();
  ExpectInitialState();

  // Keyboard
  capture_window_.MouseMoved(x, y, false, false, false);
  capture_window_.KeyPressed('W', false, false, false);
  capture_window_.PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kRight);

  capture_window_.KeyPressed('S', false, false, false);
  capture_window_.PreRender();
  ExpectInitialState();
}

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksAtTheLeftOfTimeline) {
  capture_window_.PreRender();
  ExpectInitialState();

  const Vec2i kTimelinePos = capture_window_.GetViewport().WorldToScreen(
      capture_window_.GetTimeGraph()->GetTimelineUi()->GetPos());

  int x = kTimelinePos[0];
  int y = kTimelinePos[1];

  capture_window_.MouseWheelMoved(x, y, 1, /*ctrl=*/false);
  capture_window_.PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kLeft);

  capture_window_.MouseWheelMoved(x, y, -1, /*ctrl=*/false);
  capture_window_.PreRender();
  ExpectInitialState();

  // Keyboard
  capture_window_.MouseMoved(x, y, false, false, false);
  capture_window_.KeyPressed('W', false, false, false);
  capture_window_.PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kLeft);

  capture_window_.KeyPressed('S', false, false, false);
  capture_window_.PreRender();
  ExpectInitialState();
}

TEST_F(NavigationTestCaptureWindow, VerticalZoomWorksAsExpected) {
  capture_window_.PreRender();
  ExpectInitialState();

  const Vec2i kTimeGraphSize =
      capture_window_.GetViewport().WorldToScreen(capture_window_.GetTimeGraph()->GetSize());
  int x = kTimeGraphSize[0] / 2;
  int y = kTimeGraphSize[1] - kBottomSafetyMargin;
  capture_window_.MouseMoved(x, y, /*left=*/false, /*right=*/false, /*middle=*/false);

  float old_height =
      capture_window_.GetTimeGraph()->GetTrackContainer()->GetVisibleTracksTotalHeight();
  capture_window_.KeyPressed('+', /*ctrl=*/true, /*shift=*/false, /*alt=*/false);
  capture_window_.PreRender();
  EXPECT_GT(capture_window_.GetTimeGraph()->GetTrackContainer()->GetVisibleTracksTotalHeight(),
            old_height);

  capture_window_.KeyPressed('-', /*ctrl=*/true, /*shift=*/false, /*alt=*/false);
  capture_window_.PreRender();
  EXPECT_EQ(capture_window_.GetTimeGraph()->GetTrackContainer()->GetVisibleTracksTotalHeight(),
            old_height);
}

TEST_F(NavigationTestCaptureWindow, PanTimeWorksAsExpected) {
  // TODO (b/226386133): Extend this test
  capture_window_.PreRender();
  ExpectInitialState();
  constexpr double kEpsilon = 1e-9;

  int x = 0;
  int y = capture_window_.GetViewport().GetScreenHeight() - kBottomSafetyMargin;

  // Pan time - need to zoom in a bit first, then pan slighty right and back again
  capture_window_.MouseMoved(x, y, false, false, false);
  for (int i = 0; i < 10; i++) {
    capture_window_.KeyPressed('W', false, false, false);
  }

  EXPECT_DOUBLE_EQ(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);

  capture_window_.KeyPressed('D', false, false, false);
  capture_window_.PreRender();
  EXPECT_GT(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);
  EXPECT_GT(capture_window_.GetTimeGraph()->GetMinTimeUs(), 0.0);

  capture_window_.KeyPressed('A', false, false, false);
  capture_window_.PreRender();
  EXPECT_NEAR(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0, kEpsilon);
  EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMinTimeUs(), 0.0, kEpsilon);

  const int kRightArrowKeyCode = 20;
  const int kLeftArrowKeyCode = 18;
  capture_window_.KeyPressed(kRightArrowKeyCode, false, false, false);
  capture_window_.PreRender();
  EXPECT_GT(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0);
  EXPECT_GT(capture_window_.GetTimeGraph()->GetMinTimeUs(), 0.0);

  double min_time_us_after_right_arrow = capture_window_.GetTimeGraph()->GetMinTimeUs();
  capture_window_.KeyPressed(kRightArrowKeyCode, false, false, false);
  capture_window_.KeyPressed(kLeftArrowKeyCode, false, false, false);
  // A right arrow key pressed followed by a left one should return to the original position.
  EXPECT_NEAR(min_time_us_after_right_arrow, capture_window_.GetTimeGraph()->GetMinTimeUs(),
              kEpsilon);

  capture_window_.KeyPressed(kLeftArrowKeyCode, false, false, false);
  capture_window_.PreRender();
  EXPECT_NEAR(capture_window_.GetTimeGraph()->GetHorizontalSlider()->GetPosRatio(), 0.0, kEpsilon);
  EXPECT_NEAR(capture_window_.GetTimeGraph()->GetMinTimeUs(), 0.0, kEpsilon);
}

TEST_F(NavigationTestCaptureWindow, Scrolling) {
  capture_window_.PreRender();
  ExpectInitialState();

  int x = 0;
  int y = capture_window_.GetViewport().GetScreenHeight() - kBottomSafetyMargin;

  // Mouse Wheel should scroll up and down.
  capture_window_.MouseWheelMoved(x, y, -1, /*ctrl=*/false);
  capture_window_.PreRender();
  ExpectScrolledDownFromInitialState();

  capture_window_.MouseWheelMoved(x, y, 1, /*ctrl=*/false);
  capture_window_.PreRender();
  ExpectInitialState();

  // Down
  capture_window_.KeyPressed(21, false, false, false);
  capture_window_.PreRender();
  ExpectScrolledDownFromInitialState();

  // Up
  capture_window_.KeyPressed(19, false, false, false);
  capture_window_.PreRender();
  ExpectInitialState();

  // Page Down
  capture_window_.KeyPressed(23, false, false, false);
  capture_window_.PreRender();
  ExpectScrolledDownFromInitialState();

  // Page Up
  capture_window_.KeyPressed(22, false, false, false);
  capture_window_.PreRender();
  ExpectInitialState();
}

}  // namespace orbit_gl