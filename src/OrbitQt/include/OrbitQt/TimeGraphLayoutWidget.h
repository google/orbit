// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TIME_GRAPH_LAYOUT_WIDGET_H_
#define TIME_GRAPH_LAYOUT_WIDGET_H_

#include <stdint.h>

#include <QObject>
#include <QString>
#include <QWidget>
#include <cmath>
#include <string_view>

#include "ConfigWidgets/PropertyConfigWidget.h"
#include "OrbitGl/TimeGraphLayout.h"

namespace orbit_qt {

class TimeGraphLayoutWidget : public orbit_config_widgets::PropertyConfigWidget,
                              public TimeGraphLayout {
  Q_OBJECT
 public:
  explicit TimeGraphLayoutWidget(QWidget* parent = nullptr);

  [[nodiscard]] float GetTextBoxHeight() const override {
    return text_box_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetTextCoresHeight() const override {
    return core_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetThreadStateTrackHeight() const override {
    return thread_state_track_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetEventTrackHeightFromTid(uint32_t tid) const override;
  [[nodiscard]] float GetTrackContentBottomMargin() const override {
    return track_content_bottom_margin_.value() * scale_.value();
  }
  [[nodiscard]] float GetTrackContentTopMargin() const override {
    return track_content_top_margin_.value() * scale_.value();
  }
  [[nodiscard]] float GetTrackLabelOffsetX() const override {
    return track_label_offset_x_.value();
  }
  [[nodiscard]] float GetSliderWidth() const override {
    return slider_width_.value() * scale_.value();
  }
  [[nodiscard]] float GetMinSliderLength() const override {
    return min_slider_length_.value() * scale_.value();
  }
  [[nodiscard]] float GetSliderResizeMargin() const override;
  [[nodiscard]] float GetTimeBarHeight() const override {
    return time_bar_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetTrackTabWidth() const override { return track_tab_width_.value(); }
  [[nodiscard]] float GetTrackTabHeight() const override {
    return track_tab_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetTrackTabOffset() const override { return track_tab_offset_.value(); }
  [[nodiscard]] float GetTrackIndentOffset() const override { return track_indent_offset_.value(); }
  [[nodiscard]] float GetCollapseButtonSize(int indentation_level) const override;
  [[nodiscard]] float GetCollapseButtonOffset() const override {
    return collapse_button_offset_.value();
  }
  [[nodiscard]] float GetRoundingRadius() const override {
    return rounding_radius_.value() * scale_.value();
  }
  [[nodiscard]] float GetRoundingNumSides() const override { return rounding_num_sides_.value(); }
  [[nodiscard]] float GetTextOffset() const override {
    return text_offset_.value() * scale_.value();
  }
  [[nodiscard]] float GetLeftMargin() const override {
    return left_margin_.value() * scale_.value();
  }
  [[nodiscard]] float GetRightMargin() const override {
    return right_margin_.value() * scale_.value();
  }
  [[nodiscard]] float GetMinButtonSize() const override { return min_button_size_.value(); }
  [[nodiscard]] float GetButtonWidth() const override {
    return button_width_.value() * scale_.value();
  }
  [[nodiscard]] float GetButtonHeight() const override {
    return button_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetSpaceBetweenTracks() const override {
    return space_between_tracks_.value() * scale_.value();
  }
  [[nodiscard]] float GetSpaceBetweenTracksAndTimeline() const override {
    return space_between_tracks_and_timeline_.value() * scale_.value();
  }
  [[nodiscard]] float GetSpaceBetweenCores() const override {
    return space_between_cores_.value() * scale_.value();
  }
  [[nodiscard]] float GetSpaceBetweenGpuDepths() const override {
    return space_between_gpu_depths_.value() * scale_.value();
  }
  [[nodiscard]] float GetSpaceBetweenThreadPanes() const override {
    return space_between_thread_panes_.value() * scale_.value();
  }
  [[nodiscard]] float GetSpaceBetweenSubtracks() const override {
    return space_between_subtracks_.value() * scale_.value();
  }
  [[nodiscard]] float GetGenericFixedSpacerWidth() const override {
    return generic_fixed_spacer_width_.value();
  }
  [[nodiscard]] float GetThreadDependencyArrowHeadWidth() const override {
    return thread_dependency_arrow_head_width_.value() * scale_.value();
  }
  [[nodiscard]] float GetThreadDependencyArrowHeadHeight() const override {
    return thread_dependency_arrow_head_height_.value() * scale_.value();
  }
  [[nodiscard]] float GetThreadDependencyArrowBodyWidth() const override {
    return thread_dependency_arrow_body_width_.value() * scale_.value();
  }
  [[nodiscard]] float GetScale() const override { return scale_.value(); }
  void SetScale(float value) override;
  [[nodiscard]] bool GetDrawTrackBackground() const override {
    return draw_track_background_.value();
  }
  [[nodiscard]] uint32_t GetFontSize() const override {
    return std::lround(static_cast<float>(font_size_.value()) * scale_.value());
  }

  [[nodiscard]] int GetMaxLayoutingLoops() const override { return max_layouting_loops_.value(); }

 private:
  FloatProperty text_box_height_{{
      .initial_value = 20.f,
      .label = "Text Box Height:",
  }};

  FloatProperty core_height_{{
      .initial_value = 10.f,
      .label = "Core Height:",
  }};

  FloatProperty thread_state_track_height_{{
      .initial_value = 6.f,
      .label = "Thread State Track Height:",
  }};

  FloatProperty event_track_height_{{
      .initial_value = 10.f,
      .label = "Event Track Height:",
  }};
  FloatProperty all_threads_event_track_scale_{
      {.initial_value = 2.f, .max = 10.f, .label = "All Threads Event Track Scale:"}};
  FloatProperty variable_track_height_{{
      .initial_value = 20.f,
      .label = "Variable Track Height:",
  }};
  FloatProperty track_content_bottom_margin_{{
      .initial_value = 5.f,
      .label = "Track Content Bottom Margin:",
  }};
  FloatProperty track_content_top_margin_{{
      .initial_value = 5.f,
      .label = "Track Content Top Margin:",
  }};
  FloatProperty space_between_cores_{{
      .initial_value = 2.f,
      .label = "Space between Cores:",
  }};
  FloatProperty space_between_gpu_depths_{{
      .initial_value = 2.f,
      .label = "Space between GPU depths:",
  }};
  FloatProperty space_between_tracks_{{
      .initial_value = 10.f,
      .label = "Space between Tracks:",
  }};
  FloatProperty space_between_tracks_and_timeline_{{
      .initial_value = 10.f,
      .label = "Space between Tracks and Timeline:",
  }};
  FloatProperty space_between_thread_panes_{{
      .initial_value = 5.f,
      .label = "Space between Thread Panes:",
  }};
  FloatProperty space_between_subtracks_{{
      .initial_value = 0.f,
      .label = "Space between Subtracks:",
  }};
  FloatProperty track_label_offset_x_{{
      .initial_value = 30.f,
      .label = "Track Label x-offset:",
  }};
  FloatProperty slider_width_{{
      .initial_value = 15.f,
      .label = "Slider Width:",
  }};
  FloatProperty min_slider_length_{{
      .initial_value = 20.f,
      .label = "Minimum Slider Length:",
  }};
  FloatProperty track_tab_width_{{
      .initial_value = 350.f,
      .min = 0.f,
      .max = 1000.f,
      .label = "Track Tab Width:",
  }};
  FloatProperty track_tab_height_{{
      .initial_value = 25.f,
      .label = "Track Tab Height:",
  }};
  FloatProperty track_tab_offset_{{
      .initial_value = 0.f,
      .label = "Track Tab Offset:",
  }};
  FloatProperty track_indent_offset_{{
      .initial_value = 5.f,
      .label = "Track Indent Offset:",
  }};
  FloatProperty collapse_button_offset_{{
      .initial_value = 15.f,
      .label = "Collapse Button Offset:",
  }};
  FloatProperty collapse_button_size_{{
      .initial_value = 10.f,
      .label = "Collapse Button Size:",
  }};
  FloatProperty collapse_button_decrease_per_indentation_{{
      .initial_value = 2.f,
      .label = "Collapse Button decrease per indentation:",
  }};
  FloatProperty rounding_radius_{{
      .initial_value = 8.f,
      .label = "Rounding Radius:",
  }};
  FloatProperty rounding_num_sides_{{.initial_value = 16, .label = "Rounding Num Sides:"}};
  FloatProperty text_offset_{{
      .initial_value = 5.f,
      .label = "Text Offset:",
  }};
  FloatProperty left_margin_{{
      .initial_value = 0.f,
      .min = 0.f,
      .max = 1000.f,
      .label = "Left Margin:",
  }};
  FloatProperty right_margin_{{
      .initial_value = 10.f,
      .label = "Right Margin:",
  }};
  FloatProperty min_button_size_{{
      .initial_value = 5.f,
      .label = "Min Button Size:",
  }};

  constexpr static float kMinButtonSize = 5.f;
  constexpr static float kMaxButtonSize = 50.f;
  FloatProperty button_width_{{
      .initial_value = 15.f,
      .min = kMinButtonSize,
      .max = kMaxButtonSize,
      .label = "Button Width:",
  }};
  FloatProperty button_height_{{
      .initial_value = 15.f,
      .min = kMinButtonSize,
      .max = kMaxButtonSize,
      .label = "Button Height:",
  }};

  FloatProperty generic_fixed_spacer_width_{{
      .initial_value = 10.f,
      .label = "Generic fixed Spacer width:",
  }};

  constexpr static float kMinScale = .1f;
  constexpr static float kMaxScale = 3.f;
  FloatProperty scale_{
      {.initial_value = 1.f, .min = kMinScale, .max = kMaxScale, .label = "Scale:"}};

  FloatProperty time_bar_height_{{
      .initial_value = 30.f,
      .label = "Time Bar Height:",
  }};
  IntProperty font_size_{{
      .initial_value = 14,
      .label = "Font Size:",
  }};
  IntProperty thread_dependency_arrow_head_width_{{
      .initial_value = 16,
      .label = "Thread Dependency Arrow Head Width:",
  }};
  IntProperty thread_dependency_arrow_head_height_{{
      .initial_value = 15,
      .label = "Thread Dependency Arrow Head Height:",
  }};
  IntProperty thread_dependency_arrow_body_width_{{
      .initial_value = 4,
      .label = "Thread Dependency Arrow Head Width:",
  }};
  BoolProperty draw_track_background_{{.initial_value = true, .label = "Draw Track Background"}};
  IntProperty max_layouting_loops_{
      {.initial_value = 10, .min = 1, .max = 100, .label = "Max layouting loops:"}};
};

}  // namespace orbit_qt

#endif  // TIME_GRAPH_LAYOUT_WIDGET_H_