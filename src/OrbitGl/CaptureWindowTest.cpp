// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <random>

#include "CaptureClient/AppInterface.h"
#include "CaptureWindow.h"
#include "TrackTestData.h"

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

class NavigationTestCaptureWindow : public CaptureWindow, public testing::Test {
 public:
  explicit NavigationTestCaptureWindow() : CaptureWindow(nullptr) {
    capture_client_app_ = &capture_client_app_fake_;

    Resize(kViewportWidth, kViewportHeight);

    capture_data_ = TrackTestData::GenerateTestCaptureData();
    time_graph_ = std::make_unique<TimeGraph>(this, nullptr, &viewport_, capture_data_.get(),
                                              &picking_manager_);

    AddTimers();
    PreRender();
    time_graph_->ZoomAll();

    // Make sure our expectations about the height of the timeline are correct,
    // otherwise zooming may not work as expected - if this fails, the overall
    // viewport height should be changed and the test adjusted accordingly
    ORBIT_CHECK(time_graph_->GetTimelineUi()->GetHeight() < kViewportHeight - kBottomSafetyMargin);
    ORBIT_CHECK(time_graph_->GetTimelineUi()->GetPos()[0] == 0);
  }

 protected:
  void ExpectInitialState(bool allow_small_imprecision = false) {
    EXPECT_LT(time_graph_->GetVerticalSlider()->GetLengthRatio(), 1.0);
    EXPECT_DOUBLE_EQ(time_graph_->GetVerticalSlider()->GetPosRatio(), 0.0);

    EXPECT_DOUBLE_EQ(time_graph_->GetHorizontalSlider()->GetLengthRatio(), 1.0);
    EXPECT_DOUBLE_EQ(time_graph_->GetHorizontalSlider()->GetPosRatio(), 0.0);

    if (allow_small_imprecision) {
      EXPECT_NEAR(time_graph_->GetMaxTimeUs(), time_graph_->GetCaptureMax() / 1000, kTimeEpsilonUs);
      EXPECT_NEAR(time_graph_->GetMinTimeUs(), 0, kTimeEpsilonUs);
    } else {
      EXPECT_DOUBLE_EQ(time_graph_->GetMaxTimeUs() * 1000, time_graph_->GetCaptureMax());
      EXPECT_DOUBLE_EQ(time_graph_->GetMinTimeUs() * 1000, 0);
    }
  }

  enum struct PosWithinCapture { kLeft, kRight, kMiddle, kAnywhere };
  void ExpectIsHorizontallyZoomedIn(PosWithinCapture pos) {
    EXPECT_LT(time_graph_->GetHorizontalSlider()->GetLengthRatio(), 1.0);
    EXPECT_LT(time_graph_->GetMaxTimeUs() - time_graph_->GetMinTimeUs(),
              time_graph_->GetCaptureTimeSpanUs());

    switch (pos) {
      case PosWithinCapture::kLeft:
        EXPECT_DOUBLE_EQ(time_graph_->GetHorizontalSlider()->GetPosRatio(), 0.0);
        EXPECT_LT(time_graph_->GetMaxTimeUs() * 1000, time_graph_->GetCaptureMax());
        EXPECT_DOUBLE_EQ(time_graph_->GetMinTimeUs() * 1000, 0);
        break;
      case PosWithinCapture::kRight:
        // TODO (b/226376252): This should also check if pos + size == 100%
        EXPECT_GT(time_graph_->GetHorizontalSlider()->GetPosRatio(), 0.0);
        EXPECT_DOUBLE_EQ(time_graph_->GetMaxTimeUs() * 1000, time_graph_->GetCaptureMax());
        EXPECT_GT(time_graph_->GetMinTimeUs() * 1000, 0);
        break;
      case PosWithinCapture::kMiddle:
        EXPECT_NEAR(time_graph_->GetHorizontalSlider()->GetPosRatio(), 0.5, 0.01);
        EXPECT_NEAR(time_graph_->GetCaptureMax() - time_graph_->GetMaxTimeUs() * 1000,
                    time_graph_->GetMinTimeUs() * 1000 - time_graph_->GetCaptureMin(), 1);
        break;
      case PosWithinCapture::kAnywhere:
        // Covered by basic conditions above
        break;
    }
  }

 private:
  CaptureClientAppInterfaceFake capture_client_app_fake_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;

  void AddTimers() {
    auto timers = TrackTestData::GenerateTimers();
    for (auto& timer : timers) {
      time_graph_->ProcessTimer(timer, nullptr);
    }
  }
};

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksInTheMiddleOfTimeline) {
  PreRender();
  ExpectInitialState();
  TimelineUi* timeline_ui = time_graph_->GetTimelineUi();
  const Vec2i kTimelineSize = viewport_.WorldToScreen(timeline_ui->GetSize());
  const Vec2i kTimelinePos = viewport_.WorldToScreen(timeline_ui->GetPos());

  int x = kTimelinePos[0] + kTimelineSize[0] / 2;
  int y = kTimelinePos[1] + kTimelineSize[1] / 2;

  MouseWheelMoved(x, y, 1, false);
  PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kMiddle);

  MouseWheelMoved(x, y, -1, false);
  PreRender();
  ExpectInitialState();

  // Keyboard
  MouseMoved(x, y, false, false, false);
  KeyPressed('W', false, false, false);
  PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kMiddle);

  KeyPressed('S', false, false, false);
  PreRender();
  ExpectInitialState();
}

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksAtRandomPositions) {
  PreRender();
  ExpectInitialState();
  const Vec2i kTimelineSize = viewport_.WorldToScreen(time_graph_->GetTimelineUi()->GetSize());

  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, kTimelineSize[0]);

  constexpr int kNumTries = 100;
  for (int i = 0; i < kNumTries; ++i) {
    const int x = dist(mt);
    const int y = kTimelineSize[1] - kBottomSafetyMargin;

    // Zoom in twice, then zoom out twice. Check that the intermediate states match
    MouseWheelMoved(x, y, 1, true);
    PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    float last_slider_pos = time_graph_->GetHorizontalSlider()->GetPosRatio();
    double last_min_time = time_graph_->GetMinTimeUs();
    double last_max_time = time_graph_->GetMaxTimeUs();
    MouseWheelMoved(x, y, 1, true);
    PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    MouseWheelMoved(x, y, -1, true);
    PreRender();
    EXPECT_NEAR(time_graph_->GetHorizontalSlider()->GetPosRatio(), last_slider_pos,
                kSliderPosEpsilon);
    EXPECT_NEAR(time_graph_->GetMinTimeUs(), last_min_time, kTimeEpsilonUs);
    EXPECT_NEAR(time_graph_->GetMaxTimeUs(), last_max_time, kTimeEpsilonUs);

    MouseWheelMoved(x, y, -1, true);
    PreRender();
    ExpectInitialState(true);

    // Same test using the Keyboard
    MouseMoved(x, y, false, false, false);
    KeyPressed('W', false, false, false);
    PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    last_slider_pos = time_graph_->GetHorizontalSlider()->GetPosRatio();
    last_min_time = time_graph_->GetMinTimeUs();
    last_max_time = time_graph_->GetMaxTimeUs();
    KeyPressed('W', false, false, false);
    PreRender();
    ExpectIsHorizontallyZoomedIn(PosWithinCapture::kAnywhere);

    KeyPressed('S', false, false, false);
    PreRender();
    EXPECT_NEAR(time_graph_->GetHorizontalSlider()->GetPosRatio(), last_slider_pos,
                kSliderPosEpsilon);
    EXPECT_NEAR(time_graph_->GetMinTimeUs(), last_min_time, kTimeEpsilonUs);
    EXPECT_NEAR(time_graph_->GetMaxTimeUs(), last_max_time, kTimeEpsilonUs);

    KeyPressed('S', false, false, false);
    PreRender();
    ExpectInitialState(true);
  }
}

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksAtTheRightOfTimeline) {
  PreRender();
  ExpectInitialState();

  TimelineUi* timeline_ui = time_graph_->GetTimelineUi();
  const Vec2i kTimelineSize = viewport_.WorldToScreen(timeline_ui->GetSize());
  const Vec2i kTimelinePos = viewport_.WorldToScreen(timeline_ui->GetPos());

  int x = kTimelinePos[0] + kTimelineSize[0];
  int y = kTimelinePos[1] + kTimelineSize[1] / 2;

  MouseWheelMoved(x, y, 1, true);
  PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kRight);

  MouseWheelMoved(x, y, -1, true);
  PreRender();
  ExpectInitialState();

  // Keyboard
  MouseMoved(x, y, false, false, false);
  KeyPressed('W', false, false, false);
  PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kRight);

  KeyPressed('S', false, false, false);
  PreRender();
  ExpectInitialState();
}

TEST_F(NavigationTestCaptureWindow, ZoomTimeWorksAtTheLeftOfTimeline) {
  PreRender();
  ExpectInitialState();

  const Vec2i kTimelinePos = viewport_.WorldToScreen(time_graph_->GetTimelineUi()->GetPos());

  int x = kTimelinePos[0];
  int y = kTimelinePos[1];

  MouseWheelMoved(x, y, 1, false);
  PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kLeft);

  MouseWheelMoved(x, y, -1, false);
  PreRender();
  ExpectInitialState();

  // Keyboard
  MouseMoved(x, y, false, false, false);
  KeyPressed('W', false, false, false);
  PreRender();
  ExpectIsHorizontallyZoomedIn(PosWithinCapture::kLeft);

  KeyPressed('S', false, false, false);
  PreRender();
  ExpectInitialState();
}

TEST_F(NavigationTestCaptureWindow, VerticalZoomWorksAsExpected) {
  PreRender();
  ExpectInitialState();

  const Vec2i kTimeGraphSize = viewport_.WorldToScreen(time_graph_->GetSize());
  int x = kTimeGraphSize[0] / 2;
  int y = kTimeGraphSize[1] - kBottomSafetyMargin;
  MouseMoved(x, y, /*left=*/false, /*right=*/false, /*middle=*/false);

  // float old_height = time_graph_->GetTrackContainer()->GetVisibleTracksTotalHeight();
  KeyPressed('+', /*ctrl=*/true, /*shift=*/false, /*alt=*/false);
  PreRender();
  // TODO(b/168101815): Add ctrl + '+' for VerticalZoom.
  // EXPECT_GT(time_graph_->GetTrackContainer()->GetVisibleTracksTotalHeight(), old_height);

  KeyPressed('-', /*ctrl=*/true, /*shift=*/false, /*alt=*/false);
  PreRender();
  // TODO(b/168101815): Add ctrl + '-' for VerticalZoom.
  // EXPECT_EQ(time_graph_->GetTrackContainer()->GetVisibleTracksTotalHeight(), old_height);
}

TEST_F(NavigationTestCaptureWindow, PanTimeWorksAsExpected) {
  // TODO (b/226386133): Extend this test
  PreRender();
  ExpectInitialState();

  int x = 0;
  int y = viewport_.GetScreenHeight() - kBottomSafetyMargin;

  // Pan time - need to zoom in a bit first, then pan slighty right and back again
  MouseMoved(x, y, false, false, false);
  KeyPressed('W', false, false, false);
  KeyPressed('D', false, false, false);
  PreRender();
  EXPECT_GT(time_graph_->GetHorizontalSlider()->GetPosRatio(), 0.0);
  EXPECT_GT(time_graph_->GetMinTimeUs(), 0.0);

  KeyPressed('A', false, false, false);
  PreRender();
  EXPECT_EQ(time_graph_->GetHorizontalSlider()->GetPosRatio(), 0.0);
  EXPECT_EQ(time_graph_->GetMinTimeUs(), 0.0);
}

}  // namespace orbit_gl