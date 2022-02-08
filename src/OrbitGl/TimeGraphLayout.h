// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_TIME_GRAPH_LAYOUT_H_

#include <math.h>
#include <stdint.h>

#include <algorithm>

#include "OrbitBase/ThreadUtils.h"

class TimeGraphLayout {
 public:
  TimeGraphLayout();

  const float kMinScale = 0.333f;
  const float kMaxScale = 3.f;

  float GetTextBoxHeight() const { return text_box_height_ * scale_; }
  float GetTextCoresHeight() const { return core_height_ * scale_; }
  float GetThreadStateTrackHeight() const { return thread_state_track_height_ * scale_; }
  float GetEventTrackHeightFromTid(uint32_t tid = orbit_base::kInvalidThreadId) const;
  float GetVariableTrackHeight() const { return variable_track_height_ * scale_; }
  float GetTrackContentBottomMargin() const { return track_content_bottom_margin_ * scale_; }
  float GetTrackContentTopMargin() const { return track_content_top_margin_ * scale_; }
  float GetTrackLabelOffsetX() const { return track_label_offset_x_; }
  float GetSliderWidth() const { return slider_width_; }
  float GetTimeBarHeight() const { return time_bar_height_; }
  float GetTimeBarMargin() const { return time_bar_margin_; }
  float GetTrackTabWidth() const { return track_tab_width_; }
  float GetTrackTabHeight() const { return track_tab_height_ * scale_; }
  float GetTrackTabOffset() const { return track_tab_offset_; }
  float GetTrackIndentOffset() const { return track_indent_offset_; }
  float GetCollapseButtonSize(int indentation_level) const;
  float GetCollapseButtonOffset() const { return collapse_button_offset_; }
  float GetRoundingRadius() const { return rounding_radius_ * scale_; }
  float GetRoundingNumSides() const { return rounding_num_sides_; }
  float GetTextOffset() const { return text_offset_ * scale_; }
  float GetBottomMargin() const;
  float GetTracksTopMargin() const { return GetTimeBarHeight() + GetTimeBarMargin(); }
  float GetRightMargin() const { return right_margin_; }
  float GetSpaceBetweenTracks() const { return space_between_tracks_ * scale_; }
  float GetSpaceBetweenCores() const { return space_between_cores_ * scale_; }
  float GetSpaceBetweenGpuDepths() const { return space_between_gpu_depths_ * scale_; }
  float GetSpaceBetweenThreadPanes() const { return space_between_thread_panes_ * scale_; }
  float GetSpaceBetweenSubtracks() const { return space_between_subtracks_ * scale_; }
  float GetToolbarIconHeight() const { return toolbar_icon_height_; }
  float GetGenericFixedSpacerWidth() const { return generic_fixed_spacer_width_; }
  float GetScale() const { return scale_; }
  void SetScale(float value) { scale_ = std::clamp(value, kMinScale, kMaxScale); }
  void SetDrawProperties(bool value) { draw_properties_ = value; }
  bool DrawProperties();
  bool GetDrawTrackBackground() const { return draw_track_background_; }
  uint32_t GetFontSize() const { return font_size_; }
  [[nodiscard]] uint32_t CalculateZoomedFontSize() const {
    return lround(GetFontSize() * GetScale());
  }

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
  float time_bar_height_;
  float time_bar_margin_;
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

  uint32_t font_size_;

  float space_between_cores_;
  float space_between_gpu_depths_;
  float space_between_tracks_;
  float space_between_thread_panes_;
  float space_between_subtracks_;
  float generic_fixed_spacer_width_;

  float toolbar_icon_height_;
  float scale_;

  bool draw_properties_ = false;
  bool draw_track_background_ = true;

 private:
  float GetEventTrackHeight() const { return event_track_height_ * scale_; }
  float GetAllThreadsEventTrackScale() const { return all_threads_event_track_scale_; }
};

#endif  // ORBIT_GL_TIME_GRAPH_LAYOUT_H_
