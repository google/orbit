// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_STATIC_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_STATIC_TIME_GRAPH_LAYOUT_H_

#include <stdint.h>

#include <algorithm>
#include <cmath>

#include "OrbitBase/ThreadConstants.h"
#include "TimeGraphLayout.h"

namespace orbit_gl {
class StaticTimeGraphLayout : public TimeGraphLayout {
 public:
  constexpr static float kMinScale = 0.333f;
  constexpr static float kMaxScale = 3.f;

  [[nodiscard]] float GetTextBoxHeight() const override { return text_box_height_ * scale_; }
  [[nodiscard]] float GetTextCoresHeight() const override { return core_height_ * scale_; }
  [[nodiscard]] float GetThreadStateTrackHeight() const override {
    return thread_state_track_height_ * scale_;
  }
  [[nodiscard]] float GetEventTrackHeightFromTid(uint32_t tid) const override {
    float height = event_track_height_ * scale_;
    if (tid == orbit_base::kAllProcessThreadsTid) {
      height *= all_threads_event_track_scale_;
    }
    return height;
  }
  [[nodiscard]] float GetVariableTrackHeight() const override {
    return variable_track_height_ * scale_;
  }
  [[nodiscard]] float GetTrackContentBottomMargin() const override {
    return track_content_bottom_margin_ * scale_;
  }
  [[nodiscard]] float GetTrackContentTopMargin() const override {
    return track_content_top_margin_ * scale_;
  }
  [[nodiscard]] float GetTrackLabelOffsetX() const override { return track_label_offset_x_; }
  [[nodiscard]] float GetSliderWidth() const override { return slider_width_ * scale_; }
  [[nodiscard]] float GetMinSliderLength() const override { return min_slider_length_ * scale_; }
  [[nodiscard]] float GetSliderResizeMargin() const override {
    // The resize part of the slider is 1/3 of the min length.
    constexpr float kRatioMinSliderLengthResizePart = 3.f;
    return GetMinSliderLength() / kRatioMinSliderLengthResizePart;
  }
  [[nodiscard]] float GetTimeBarHeight() const override { return time_bar_height_ * scale_; }
  [[nodiscard]] float GetTrackTabWidth() const override { return track_tab_width_; }
  [[nodiscard]] float GetTrackTabHeight() const override { return track_tab_height_ * scale_; }
  [[nodiscard]] float GetTrackTabOffset() const override { return track_tab_offset_; }
  [[nodiscard]] float GetTrackIndentOffset() const override { return track_indent_offset_; }
  [[nodiscard]] float GetCollapseButtonSize(int indentation_level) const override {
    float button_size_without_scaling =
        collapse_button_size_ -
        collapse_button_decrease_per_indentation_ * static_cast<float>(indentation_level);

    // We want the button to scale slower than other elements, so we use sqrt() function.
    return button_size_without_scaling * std::sqrt(scale_);
  }
  [[nodiscard]] float GetCollapseButtonOffset() const override { return collapse_button_offset_; }
  [[nodiscard]] float GetRoundingRadius() const override { return rounding_radius_ * scale_; }
  [[nodiscard]] float GetRoundingNumSides() const override { return rounding_num_sides_; }
  [[nodiscard]] float GetTextOffset() const override { return text_offset_ * scale_; }
  [[nodiscard]] float GetRightMargin() const override { return right_margin_ * scale_; }
  [[nodiscard]] float GetMinButtonSize() const override { return min_button_size_; }
  [[nodiscard]] float GetButtonWidth() const override { return button_width_ * scale_; }
  [[nodiscard]] float GetButtonHeight() const override { return button_height_ * scale_; }
  [[nodiscard]] float GetSpaceBetweenTracks() const override {
    return space_between_tracks_ * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenTracksAndTimeline() const override {
    return space_between_tracks_and_timeline_ * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenCores() const override {
    return space_between_cores_ * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenGpuDepths() const override {
    return space_between_gpu_depths_ * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenThreadPanes() const override {
    return space_between_thread_panes_ * scale_;
  }
  [[nodiscard]] float GetSpaceBetweenSubtracks() const override {
    return space_between_subtracks_ * scale_;
  }
  [[nodiscard]] float GetGenericFixedSpacerWidth() const override {
    return generic_fixed_spacer_width_;
  }
  [[nodiscard]] float GetThreadDependencyArrowHeadWidth() const override {
    return thread_dependency_arrow_head_width_ * scale_;
  }
  [[nodiscard]] float GetThreadDependencyArrowHeadHeight() const override {
    return thread_dependency_arrow_head_height_ * scale_;
  }
  [[nodiscard]] float GetThreadDependencyArrowBodyWidth() const override {
    return thread_dependency_arrow_body_width_ * scale_;
  }
  [[nodiscard]] float GetScale() const override { return scale_; }
  void SetScale(float value) override { scale_ = std::clamp(value, kMinScale, kMaxScale); }
  [[nodiscard]] bool GetDrawTrackBackground() const override { return draw_track_background_; }
  [[nodiscard]] uint32_t GetFontSize() const override { return std::lround(font_size_ * scale_); }

  [[nodiscard]] int GetMaxLayoutingLoops() const override { return max_layouting_loops_; }

 private:
  float text_box_height_ = 20.f;
  float core_height_ = 10.f;
  float thread_state_track_height_ = 6.0f;
  float event_track_height_ = 10.f;
  float all_threads_event_track_scale_ = 2.f;
  float variable_track_height_ = 20.f;
  float track_content_bottom_margin_ = 5.f;
  float track_content_top_margin_ = 5.f;
  float space_between_cores_ = 2.f;
  float space_between_gpu_depths_ = 2.f;
  float space_between_tracks_ = 10.f;
  float space_between_tracks_and_timeline_ = 10.f;
  float space_between_thread_panes_ = 5.f;
  float space_between_subtracks_ = 0.f;
  float track_label_offset_x_ = 30.f;
  float slider_width_ = 15.f;
  float min_slider_length_ = 20.f;
  float track_tab_width_ = 350.f;
  float track_tab_height_ = 25.f;
  float track_tab_offset_ = 0.f;
  float track_indent_offset_ = 5.f;
  float collapse_button_offset_ = 15.f;
  float collapse_button_size_ = 10.f;
  float collapse_button_decrease_per_indentation_ = 2.f;
  float rounding_radius_ = 8.f;
  float rounding_num_sides_ = 16;
  float text_offset_ = 5.f;
  float right_margin_ = 10.f;
  float min_button_size_ = 5.f;
  float button_width_ = 15.f;
  float button_height_ = 15.f;
  float generic_fixed_spacer_width_ = 10.f;
  float scale_ = 1.f;
  float time_bar_height_ = 30.f;
  uint32_t font_size_ = 14;
  float thread_dependency_arrow_head_width_ = 16;
  float thread_dependency_arrow_head_height_ = 15;
  float thread_dependency_arrow_body_width_ = 4;

  bool draw_track_background_ = true;

  int max_layouting_loops_ = 10;
};
}  // namespace orbit_gl

#endif  // ORBIT_GL_STATIC_TIME_GRAPH_LAYOUT_H_
