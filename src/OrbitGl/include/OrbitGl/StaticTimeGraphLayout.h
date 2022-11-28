// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_STATIC_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_STATIC_TIME_GRAPH_LAYOUT_H_

#include <stdint.h>

#include <algorithm>
#include <cmath>

#include "OrbitBase/ThreadConstants.h"
#include "OrbitGl/TimeGraphLayout.h"

namespace orbit_gl {
// This class implements a TimeGraphLayout where all properties are static values that can't be
// changed. This class is mainly for tests that need some TimeGraphLayout.
class StaticTimeGraphLayout : public TimeGraphLayout {
 public:
  constexpr static float kMinScale = 0.333f;
  constexpr static float kMaxScale = 3.f;

  [[nodiscard]] float GetTextBoxHeight() const override { return kTextBoxHeight * scale_; }
  [[nodiscard]] float GetTextCoresHeight() const override { return kCoreHeight * scale_; }
  [[nodiscard]] float GetThreadStateTrackHeight() const override {
    return kThreadStateTrackHeight * scale_;
  }
  [[nodiscard]] float GetEventTrackHeightFromTid(uint32_t tid) const override {
    float height = kEventTrackHeight * scale_;
    if (tid == orbit_base::kAllProcessThreadsTid) {
      height *= kAllThreadsEventTrackScale;
    }
    return height;
  }
  [[nodiscard]] float GetTrackContentBottomMargin() const override {
    return kTrackContentBottomMargin * scale_;
  }
  [[nodiscard]] float GetTrackContentTopMargin() const override {
    return kTrackContentTopMargin * scale_;
  }
  [[nodiscard]] float GetTrackLabelOffsetX() const override { return kTrackLabelOffsetX; }
  [[nodiscard]] float GetSliderWidth() const override { return kSliderWidth * scale_; }
  [[nodiscard]] float GetMinSliderLength() const override { return kMinSliderLength * scale_; }
  [[nodiscard]] float GetSliderResizeMargin() const override {
    // The resize part of the slider is 1/3 of the min length.
    constexpr float kRatioMinSliderLengthResizePart = 3.f;
    return GetMinSliderLength() / kRatioMinSliderLengthResizePart;
  }
  [[nodiscard]] float GetTimeBarHeight() const override { return kTimeBarHeight * scale_; }
  [[nodiscard]] float GetTrackTabWidth() const override { return kTrackTabWidth; }
  [[nodiscard]] float GetTrackTabHeight() const override { return kTrackTabHeight * scale_; }
  [[nodiscard]] float GetTrackTabOffset() const override { return kTrackTabOffset; }
  [[nodiscard]] float GetTrackIndentOffset() const override { return kTrackIndentOffset; }
  [[nodiscard]] float GetCollapseButtonSize(int indentation_level) const override {
    float button_size_without_scaling =
        kCollapseButtonSize -
        kCollapseButtonDecreasePerIndentation * static_cast<float>(indentation_level);

    // We want the button to scale slower than other elements, so we use sqrt() function.
    return button_size_without_scaling * std::sqrt(scale_);
  }
  [[nodiscard]] float GetCollapseButtonOffset() const override { return kCollapseButtonOffset; }
  [[nodiscard]] float GetRoundingRadius() const override { return kRoundingRadius * scale_; }
  [[nodiscard]] float GetRoundingNumSides() const override { return kRoundingNumSides; }
  [[nodiscard]] float GetTextOffset() const override { return kTextOffset * scale_; }
  [[nodiscard]] float GetLeftMargin() const override { return kLeftMargin * scale_; }
  [[nodiscard]] float GetRightMargin() const override { return kRightMargin * scale_; }
  [[nodiscard]] float GetMinButtonSize() const override { return kMinButtonSize; }
  [[nodiscard]] float GetButtonWidth() const override { return kButtonWidth * scale_; }
  [[nodiscard]] float GetButtonHeight() const override { return kButtonHeight * scale_; }
  [[nodiscard]] float GetSpaceBetweenTracks() const override {
    return kSpaceBetweenTracks * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenTracksAndTimeline() const override {
    return kSpaceBetweenTracksAndTimeline * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenCores() const override { return kSpaceBetweenCores * scale_; }
  [[nodiscard]] float GetSpaceBetweenGpuDepths() const override {
    return kSpaceBetweenGpuDepths * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenThreadPanes() const override {
    return kSpaceBetweenThreadPanes * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenSubtracks() const override {
    return kSpaceBetweenSubtracks * scale_;
  }
  [[nodiscard]] float GetGenericFixedSpacerWidth() const override {
    return kGenericFixedSpacerWidth;
  }
  [[nodiscard]] float GetThreadDependencyArrowHeadWidth() const override {
    return kThreadDependencyArrowHeadWidth * scale_;
  }
  [[nodiscard]] float GetThreadDependencyArrowHeadHeight() const override {
    return kThreadDependencyArrowHeadHeight * scale_;
  }
  [[nodiscard]] float GetThreadDependencyArrowBodyWidth() const override {
    return kThreadDependencyArrowBodyWidth * scale_;
  }
  [[nodiscard]] float GetScale() const override { return scale_; }
  void SetScale(float value) override { scale_ = std::clamp(value, kMinScale, kMaxScale); }
  [[nodiscard]] bool GetDrawTrackBackground() const override { return kDrawTrackBackground; }
  [[nodiscard]] uint32_t GetFontSize() const override { return std::lround(kFontSize * scale_); }

  [[nodiscard]] int GetMaxLayoutingLoops() const override { return kMaxLayoutingLoops; }

 private:
  constexpr static float kTextBoxHeight = 20.f;
  constexpr static float kCoreHeight = 10.f;
  constexpr static float kThreadStateTrackHeight = 6.0f;
  constexpr static float kEventTrackHeight = 10.f;
  constexpr static float kAllThreadsEventTrackScale = 2.f;
  constexpr static float kVariableTrackHeight = 20.f;
  constexpr static float kTrackContentBottomMargin = 5.f;
  constexpr static float kTrackContentTopMargin = 5.f;
  constexpr static float kSpaceBetweenCores = 2.f;
  constexpr static float kSpaceBetweenGpuDepths = 2.f;
  constexpr static float kSpaceBetweenTracks = 10.f;
  constexpr static float kSpaceBetweenTracksAndTimeline = 10.f;
  constexpr static float kSpaceBetweenThreadPanes = 5.f;
  constexpr static float kSpaceBetweenSubtracks = 0.f;
  constexpr static float kTrackLabelOffsetX = 30.f;
  constexpr static float kSliderWidth = 15.f;
  constexpr static float kMinSliderLength = 20.f;
  constexpr static float kTrackTabWidth = 350.f;
  constexpr static float kTrackTabHeight = 25.f;
  constexpr static float kTrackTabOffset = 0.f;
  constexpr static float kTrackIndentOffset = 5.f;
  constexpr static float kCollapseButtonOffset = 15.f;
  constexpr static float kCollapseButtonSize = 10.f;
  constexpr static float kCollapseButtonDecreasePerIndentation = 2.f;
  constexpr static float kRoundingRadius = 8.f;
  constexpr static float kRoundingNumSides = 16;
  constexpr static float kTextOffset = 5.f;
  constexpr static float kLeftMargin = 100.f;
  constexpr static float kRightMargin = 10.f;
  constexpr static float kMinButtonSize = 5.f;
  constexpr static float kButtonWidth = 15.f;
  constexpr static float kButtonHeight = 15.f;
  constexpr static float kGenericFixedSpacerWidth = 10.f;
  constexpr static float kTimeBarHeight = 30.f;
  constexpr static uint32_t kFontSize = 14;
  constexpr static float kThreadDependencyArrowHeadWidth = 16;
  constexpr static float kThreadDependencyArrowHeadHeight = 15;
  constexpr static float kThreadDependencyArrowBodyWidth = 4;
  constexpr static bool kDrawTrackBackground = true;
  constexpr static int kMaxLayoutingLoops = 10;

  float scale_ = 1.f;
};
}  // namespace orbit_gl

#endif  // ORBIT_GL_STATIC_TIME_GRAPH_LAYOUT_H_
