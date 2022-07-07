// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimeGraphLayout.h"

#include <imgui.h>

#include "OrbitBase/ThreadConstants.h"

TimeGraphLayout::TimeGraphLayout() {
  text_box_height_ = 20.f;
  core_height_ = 10.f;
  thread_state_track_height_ = 4.0f;
  event_track_height_ = 10.f;
  all_threads_event_track_scale_ = 2.f;
  variable_track_height_ = 20.f;
  track_content_bottom_margin_ = 5.f;
  track_content_top_margin_ = 5.f;
  space_between_cores_ = 2.f;
  space_between_gpu_depths_ = 2.f;
  space_between_tracks_ = 10.f;
  space_between_tracks_and_timeline_ = 10.f;
  space_between_thread_panes_ = 5.f;
  space_between_subtracks_ = 0.f;
  track_label_offset_x_ = 30.f;
  slider_width_ = 15.f;
  min_slider_length_ = 20.f;
  track_tab_width_ = 350.f;
  track_tab_height_ = 25.f;
  track_tab_offset_ = 0.f;
  track_indent_offset_ = 5.f;
  collapse_button_offset_ = 15.f;
  collapse_button_size_ = 10.f;
  collapse_button_decrease_per_indentation_ = 2.f;
  rounding_radius_ = 8.f;
  rounding_num_sides_ = 16;
  text_offset_ = 5.f;
  right_margin_ = 10.f;
  min_button_size_ = 5.f;
  button_width_ = 15.f;
  button_height_ = 15.f;
  toolbar_icon_height_ = 24.f;
  generic_fixed_spacer_width_ = 10.f;
  scale_ = 1.f;
  time_bar_height_ = 30.f;
  font_size_ = 14;
  thread_dependency_arrow_head_width_ = 16;
  thread_dependency_arrow_head_height_ = 15;
  thread_dependency_arrow_body_width_ = 4;
};

#define FLOAT_SLIDER(x) FLOAT_SLIDER_MIN_MAX(x, 0, 100.f)
#define FLOAT_SLIDER_MIN_MAX(x, min, max)     \
  if (ImGui::SliderFloat(#x, &x, min, max)) { \
    needs_redraw = true;                      \
  }

bool TimeGraphLayout::DrawProperties() {
  bool needs_redraw = false;
  FLOAT_SLIDER(track_label_offset_x_);
  FLOAT_SLIDER(text_box_height_);
  FLOAT_SLIDER(core_height_);
  FLOAT_SLIDER(thread_state_track_height_);
  FLOAT_SLIDER(event_track_height_);
  FLOAT_SLIDER_MIN_MAX(all_threads_event_track_scale_, 0, 10.f);
  FLOAT_SLIDER(variable_track_height_);
  FLOAT_SLIDER(space_between_cores_);
  FLOAT_SLIDER(space_between_gpu_depths_);
  FLOAT_SLIDER(space_between_tracks_);
  FLOAT_SLIDER(space_between_tracks_and_timeline_);
  FLOAT_SLIDER(space_between_thread_panes_);
  FLOAT_SLIDER(space_between_subtracks_);
  FLOAT_SLIDER(slider_width_);
  FLOAT_SLIDER(time_bar_height_);
  FLOAT_SLIDER(track_tab_height_);
  FLOAT_SLIDER(track_tab_offset_);
  FLOAT_SLIDER(track_indent_offset_);
  FLOAT_SLIDER(collapse_button_size_);
  FLOAT_SLIDER(thread_dependency_arrow_head_width_);
  FLOAT_SLIDER(thread_dependency_arrow_head_height_);
  FLOAT_SLIDER(thread_dependency_arrow_body_width_);
  FLOAT_SLIDER(collapse_button_decrease_per_indentation_);

  FLOAT_SLIDER(collapse_button_offset_);
  FLOAT_SLIDER(rounding_radius_);
  FLOAT_SLIDER(rounding_num_sides_);
  FLOAT_SLIDER(text_offset_);
  FLOAT_SLIDER(right_margin_);
  FLOAT_SLIDER_MIN_MAX(button_width_, min_button_size_, 50.f);
  FLOAT_SLIDER_MIN_MAX(button_height_, min_button_size_, 50.f);
  FLOAT_SLIDER_MIN_MAX(track_tab_width_, 0, 1000.f);
  FLOAT_SLIDER_MIN_MAX(track_content_bottom_margin_, 0, 20.f);
  FLOAT_SLIDER_MIN_MAX(track_content_top_margin_, 0, 20.f);
  FLOAT_SLIDER(toolbar_icon_height_);
  FLOAT_SLIDER(generic_fixed_spacer_width_);
  FLOAT_SLIDER_MIN_MAX(scale_, kMinScale, kMaxScale);
  ImGui::Checkbox("Draw Track Background", &draw_track_background_);

  if (ImGui::SliderInt("Maximum # of layout loops", &max_layouting_loops_, 1, 100)) {
    needs_redraw = true;
  }

  return needs_redraw;
}

float TimeGraphLayout::GetCollapseButtonSize(int indentation_level) const {
  float button_size_without_scaling =
      collapse_button_size_ -
      collapse_button_decrease_per_indentation_ * static_cast<float>(indentation_level);

  // We want the button to scale slower than other elements, so we use sqrt() function.
  return button_size_without_scaling * sqrt(scale_);
}

float TimeGraphLayout::GetEventTrackHeightFromTid(uint32_t tid) const {
  float height = GetEventTrackHeight();
  if (tid == orbit_base::kAllProcessThreadsTid) {
    height *= GetAllThreadsEventTrackScale();
  }
  return height;
}

float TimeGraphLayout::GetSliderResizeMargin() const {
  // The resize part of the slider is 1/3 of the min length.
  const float kRatioMinSliderLengthResizePart = 3.f;
  return GetMinSliderLength() / kRatioMinSliderLengthResizePart;
}
