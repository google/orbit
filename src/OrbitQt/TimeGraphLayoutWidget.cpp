// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/TimeGraphLayoutWidget.h"

#include "OrbitBase/ThreadConstants.h"

namespace orbit_qt {

TimeGraphLayoutWidget::TimeGraphLayoutWidget(QWidget* parent)
    : orbit_config_widgets::PropertyConfigWidget(parent) {
  AddWidgetForProperty(&text_box_height_);
  AddWidgetForProperty(&core_height_);
  AddWidgetForProperty(&thread_state_track_height_);
  AddWidgetForProperty(&event_track_height_);
  AddWidgetForProperty(&all_threads_event_track_scale_);
  AddWidgetForProperty(&variable_track_height_);
  AddWidgetForProperty(&track_content_bottom_margin_);
  AddWidgetForProperty(&track_content_top_margin_);
  AddWidgetForProperty(&track_label_offset_x_);
  AddWidgetForProperty(&slider_width_);
  AddWidgetForProperty(&min_slider_length_);
  AddWidgetForProperty(&time_bar_height_);
  AddWidgetForProperty(&track_tab_width_);
  AddWidgetForProperty(&track_tab_height_);
  AddWidgetForProperty(&track_tab_offset_);
  AddWidgetForProperty(&track_indent_offset_);
  AddWidgetForProperty(&collapse_button_offset_);
  AddWidgetForProperty(&collapse_button_size_);
  AddWidgetForProperty(&collapse_button_decrease_per_indentation_);
  AddWidgetForProperty(&rounding_radius_);
  AddWidgetForProperty(&rounding_num_sides_);
  AddWidgetForProperty(&text_offset_);
  AddWidgetForProperty(&left_margin_);
  AddWidgetForProperty(&right_margin_);
  AddWidgetForProperty(&min_button_size_);
  AddWidgetForProperty(&button_width_);
  AddWidgetForProperty(&button_height_);
  AddWidgetForProperty(&thread_dependency_arrow_head_width_);
  AddWidgetForProperty(&thread_dependency_arrow_head_height_);
  AddWidgetForProperty(&thread_dependency_arrow_body_width_);
  AddWidgetForProperty(&scale_);
}

float TimeGraphLayoutWidget::GetCollapseButtonSize(int indentation_level) const {
  float button_size_without_scaling =
      collapse_button_size_.value() -
      collapse_button_decrease_per_indentation_.value() * static_cast<float>(indentation_level);

  // We want the button to scale slower than other elements, so we use sqrt() function.
  return button_size_without_scaling * std::sqrt(scale_.value());
}

float TimeGraphLayoutWidget::GetEventTrackHeightFromTid(uint32_t tid) const {
  float height = event_track_height_.value() * scale_.value();
  if (tid == orbit_base::kAllProcessThreadsTid) {
    height *= all_threads_event_track_scale_.value();
  }
  return height;
}

float TimeGraphLayoutWidget::GetSliderResizeMargin() const {
  // The resize part of the slider is 1/3 of the min length.
  constexpr float kRatioMinSliderLengthResizePart = 3.f;
  return GetMinSliderLength() / kRatioMinSliderLengthResizePart;
}

void TimeGraphLayoutWidget::SetScale(float value) { scale_.SetValue(value); }
}  // namespace orbit_qt
