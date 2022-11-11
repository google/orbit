// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_IM_GUI_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_IM_GUI_TIME_GRAPH_LAYOUT_H_

#include <stdint.h>

#include <algorithm>
#include <cmath>

#include "TimeGraphLayout.h"

namespace orbit_gl {
// The ImGui TimeGraphLayout is an implementation of TimeGraphLayout where all properties can be
// changed through an ImGui interface with controls for each parameter. This simplifies experimented
// with layout changes.
class ImGuiTimeGraphLayout : public TimeGraphLayout {
 public:
  ImGuiTimeGraphLayout();

  constexpr static float kMinScale = 0.333f;
  constexpr static float kMaxScale = 3.f;

  [[nodiscard]] float GetTextBoxHeight() const override { return text_box_height_ * scale_; }
  [[nodiscard]] float GetTextCoresHeight() const override { return core_height_ * scale_; }
  [[nodiscard]] float GetThreadStateTrackHeight() const override {
    return thread_state_track_height_ * scale_;
  }
  [[nodiscard]] float GetEventTrackHeightFromTid(uint32_t tid) const override;
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
  [[nodiscard]] float GetSliderResizeMargin() const override;
  [[nodiscard]] float GetTimeBarHeight() const override { return time_bar_height_ * scale_; }
  [[nodiscard]] float GetTrackTabWidth() const override { return track_tab_width_; }
  [[nodiscard]] float GetTrackTabHeight() const override { return track_tab_height_ * scale_; }
  [[nodiscard]] float GetTrackTabOffset() const override { return track_tab_offset_; }
  [[nodiscard]] float GetTrackIndentOffset() const override { return track_indent_offset_; }
  [[nodiscard]] float GetCollapseButtonSize(int indentation_level) const override;
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
  void SetDrawProperties(bool value) { draw_properties_ = value; }
  bool DrawProperties();
  [[nodiscard]] bool GetDrawTrackBackground() const override { return draw_track_background_; }
  [[nodiscard]] uint32_t GetFontSize() const override { return std::lround(font_size_ * scale_); }

  [[nodiscard]] int GetMaxLayoutingLoops() const override { return max_layouting_loops_; }

 protected:
  float text_box_height_;
  float core_height_;
  float thread_state_track_height_;
  float event_track_height_;
  float all_threads_event_track_scale_;
  float variable_track_height_;
  float track_content_bottom_margin_;
  float track_content_top_margin_;
  float track_label_offset_x_;
  float slider_width_;
  float min_slider_length_;
  float time_bar_height_;
  float track_tab_width_;
  float track_tab_height_;
  float track_tab_offset_;
  float track_indent_offset_;
  float collapse_button_offset_;
  float collapse_button_size_;
  float collapse_button_decrease_per_indentation_;
  float rounding_radius_;
  float rounding_num_sides_;
  float text_offset_;
  float right_margin_;
  float min_button_size_;
  float button_width_;
  float button_height_;
  float thread_dependency_arrow_head_width_;
  float thread_dependency_arrow_head_height_;
  float thread_dependency_arrow_body_width_;

  uint32_t font_size_;

  float space_between_cores_;
  float space_between_gpu_depths_;
  float space_between_tracks_;
  float space_between_tracks_and_timeline_;
  float space_between_thread_panes_;
  float space_between_subtracks_;
  float generic_fixed_spacer_width_;

  float toolbar_icon_height_;
  float scale_;

  bool draw_properties_ = false;
  bool draw_track_background_ = true;

  int max_layouting_loops_ = 10;

 private:
  [[nodiscard]] float GetEventTrackHeight() const { return event_track_height_ * scale_; }
  [[nodiscard]] float GetAllThreadsEventTrackScale() const {
    return all_threads_event_track_scale_;
  }
};
}  // namespace orbit_gl

#endif  // ORBIT_GL_IM_GUI_TIME_GRAPH_LAYOUT_H_
