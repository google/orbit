// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TimeGraphLayout.h"

#include <imgui.h>

TimeGraphLayout::TimeGraphLayout() {
  text_box_height_ = 20.f;
  core_height_ = 10.f;
  thread_state_track_height_ = 4.0f;
  event_track_height_ = 10.f;
  variable_track_height_ = 20.f;
  track_bottom_margin_ = 5.f;
  track_top_margin_ = 5.f;
  space_between_cores_ = 2.f;
  space_between_gpu_depths_ = 2.f;
  space_between_tracks_ = 10.f;
  space_between_tracks_and_thread_ = 5.f;
  space_between_gpu_subtracks_ = -5.f;
  space_between_thread_blocks_ = 35.f;
  track_label_offset_x_ = 30.f;
  slider_width_ = 15.f;
  track_tab_width_ = 350.f;
  track_tab_height_ = 30.f;
  track_tab_offset_ = 0.f;
  track_intent_offset_ = 5.f;
  collapse_button_offset_ = 15.f;
  rounding_radius_ = 8.f;
  rounding_num_sides_ = 16;
  text_offset_ = 5.f;
  right_margin_ = 10.f;
  scheduler_track_offset_ = 10.f;
  toolbar_icon_height_ = 24.f;
  generic_fixed_spacer_width_ = 10.f;
  scale_ = 1.f;
  time_bar_height_ = 15.f;
  font_size_ = 14;
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
  FLOAT_SLIDER(variable_track_height_);
  FLOAT_SLIDER(space_between_cores_);
  FLOAT_SLIDER(space_between_gpu_depths_);
  FLOAT_SLIDER(space_between_tracks_);
  FLOAT_SLIDER(space_between_tracks_and_thread_);
  FLOAT_SLIDER(space_between_gpu_subtracks_);
  FLOAT_SLIDER(space_between_thread_blocks_);
  FLOAT_SLIDER(slider_width_);
  FLOAT_SLIDER(time_bar_height_);
  FLOAT_SLIDER(track_tab_height_);
  FLOAT_SLIDER(track_tab_offset_);
  FLOAT_SLIDER(track_intent_offset_);
  FLOAT_SLIDER(collapse_button_offset_);
  FLOAT_SLIDER(rounding_radius_);
  FLOAT_SLIDER(rounding_num_sides_);
  FLOAT_SLIDER(text_offset_);
  FLOAT_SLIDER(right_margin_);
  FLOAT_SLIDER(scheduler_track_offset_);
  FLOAT_SLIDER_MIN_MAX(track_tab_width_, 0, 1000.f);
  FLOAT_SLIDER_MIN_MAX(track_bottom_margin_, 0, 20.f);
  FLOAT_SLIDER_MIN_MAX(track_top_margin_, 0, 20.f);
  FLOAT_SLIDER(toolbar_icon_height_);
  FLOAT_SLIDER(generic_fixed_spacer_width_);
  FLOAT_SLIDER_MIN_MAX(scale_, 0.05f, 20.f);
  ImGui::Checkbox("Draw Track Background", &draw_track_background_);

  return needs_redraw;
}

float TimeGraphLayout::GetBottomMargin() const {
  // The bottom consists of the slider (where we have to take the width, as it
  // is rotated), and the time bar.
  return GetSliderWidth() + GetTimeBarHeight();
}
