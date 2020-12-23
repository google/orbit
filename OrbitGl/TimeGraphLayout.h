// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIME_GRAPH_LAYOUT_H_
#define ORBIT_GL_TIME_GRAPH_LAYOUT_H_

class TimeGraphLayout {
 public:
  TimeGraphLayout();

  float GetTextBoxHeight() const { return text_box_height_ * scale_; }
  float GetTextCoresHeight() const { return core_height_ * scale_; }
  float GetThreadStateTrackHeight() const { return thread_state_track_height_ * scale_; }
  float GetEventTrackHeight() const { return event_track_height_ * scale_; }
  float GetGraphTrackHeight() const { return graph_track_height_ * scale_; }
  float GetTrackBottomMargin() const { return track_bottom_margin_ * scale_; }
  float GetTrackTopMargin() const { return track_top_margin_ * scale_; }
  float GetTrackLabelOffsetX() const { return track_label_offset_x_; }
  float GetSliderWidth() const { return slider_width_; }
  float GetTimeBarHeight() const { return time_bar_height_; }
  float GetTrackTabWidth() const { return track_tab_width_; }
  float GetTrackTabHeight() const { return track_tab_height_ * scale_; }
  float GetTrackTabOffset() const { return track_tab_offset_; }
  float GetCollapseButtonOffset() const { return collapse_button_offset_; }
  float GetRoundingRadius() const { return rounding_radius_ * scale_; }
  float GetRoundingNumSides() const { return rounding_num_sides_; }
  float GetTextOffset() const { return text_offset_ * scale_; }
  float GetBottomMargin() const;
  float GetTopMargin() const { return GetSchedulerTrackOffset(); }
  float GetRightMargin() const { return right_margin_; }
  float GetSchedulerTrackOffset() const { return scheduler_track_offset_; }
  float GetSpaceBetweenTracks() const { return space_between_tracks_ * scale_; }
  float GetSpaceBetweenCores() const { return space_between_cores_ * scale_; }
  float GetSpaceBetweenGpuDepths() const { return space_between_gpu_depths_ * scale_; }
  float GetSpaceBetweenTracksAndThread() const { return space_between_tracks_and_thread_ * scale_; }
  float GetToolbarIconHeight() const { return toolbar_icon_height_; }
  float GetScale() const { return scale_; }
  void SetScale(float value) { scale_ = value; }
  void SetDrawProperties(bool value) { draw_properties_ = value; }
  bool DrawProperties();
  bool GetDrawTrackBackground() const { return draw_track_background_; }

 protected:
  float text_box_height_;
  float core_height_;
  float thread_state_track_height_;
  float event_track_height_;
  float graph_track_height_;
  float track_bottom_margin_;
  float track_top_margin_;
  float track_label_offset_x_;
  float slider_width_;
  float time_bar_height_;
  float track_tab_width_;
  float track_tab_height_;
  float track_tab_offset_;
  float collapse_button_offset_;
  float rounding_radius_;
  float rounding_num_sides_;
  float text_offset_;
  float right_margin_;
  float scheduler_track_offset_;

  float space_between_cores_;
  float space_between_gpu_depths_;
  float space_between_tracks_;
  float space_between_tracks_and_thread_;
  float space_between_thread_blocks_;

  float toolbar_icon_height_;
  float scale_;

  bool draw_properties_ = false;
  bool draw_track_background_ = true;
};

#endif  // ORBIT_GL_TIME_GRAPH_LAYOUT_H_
