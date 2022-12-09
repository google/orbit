// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/BasicPageFaultsTrack.h"

#include <absl/strings/str_format.h>

#include <algorithm>
#include <functional>
#include <string_view>

#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/MultivariateTimeSeries.h"
#include "OrbitGl/PickingManager.h"

namespace orbit_gl {

[[nodiscard]] static std::array<std::string, kBasicPageFaultsTrackDimension> CreateSeriesName(
    std::string_view cgroup_name, std::string_view process_name) {
  return {absl::StrFormat("Process [%s]", process_name),
          absl::StrFormat("CGroup [%s]", cgroup_name), "System"};
}

static constexpr uint8_t kTrackValueDecimalDigits = 0;
static constexpr const char* kTrackValueUnits = "";

BasicPageFaultsTrack::BasicPageFaultsTrack(Track* parent,
                                           const orbit_gl::TimelineInfoInterface* timeline_info,
                                           orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                           std::string cgroup_name,
                                           uint64_t memory_sampling_period_ms,
                                           const orbit_client_data::ModuleManager* module_manager,
                                           const orbit_client_data::CaptureData* capture_data)
    : LineGraphTrack<kBasicPageFaultsTrackDimension>(
          parent, timeline_info, viewport, layout,
          CreateSeriesName(cgroup_name, capture_data->process_name()), kTrackValueDecimalDigits,
          kTrackValueUnits, module_manager, capture_data),
      AnnotationTrack(),
      cgroup_name_(std::move(cgroup_name)),
      memory_sampling_period_ms_(memory_sampling_period_ms),
      parent_(parent) {
  aggregation_mode_ = AggregationMode::kMax;  // Here we use Max aggregation and not summing the
                                              // values (which would also make sense) because the
                                              // code expects the max value to be known ahead of
                                              // rendering time when we do aggregation.
}

void BasicPageFaultsTrack::AddValues(
    uint64_t timestamp_ns, const std::array<double, kBasicPageFaultsTrackDimension>& values) {
  if (previous_time_and_values_.has_value()) {
    std::array<double, kBasicPageFaultsTrackDimension> differences{};
    std::transform(values.begin(), values.end(), previous_time_and_values_.value().second.begin(),
                   differences.begin(), std::minus<double>());
    series_.AddValues(previous_time_and_values_.value().first, differences);
  }

  previous_time_and_values_ = std::make_pair(timestamp_ns, values);
}

void BasicPageFaultsTrack::AddValuesAndUpdateAnnotations(
    uint64_t timestamp_ns, const std::array<double, kBasicPageFaultsTrackDimension>& values) {
  AddValues(timestamp_ns, values);

  double updated_max = GetGraphMaxValue();
  std::optional<std::pair<std::string, double>> value_upper_bound = GetValueUpperBound();
  if (!value_upper_bound.has_value() || value_upper_bound.value().second < updated_max) {
    SetValueUpperBound(
        absl::StrFormat("Maximum Rate: %.0f per %d ms", updated_max, memory_sampling_period_ms_),
        updated_max);
  }

  double updated_min = GetGraphMinValue();
  std::optional<std::pair<std::string, double>> value_lower_bound = GetValueLowerBound();
  if (!value_lower_bound.has_value() || value_lower_bound.value().second > updated_min) {
    SetValueLowerBound(
        absl::StrFormat("Minimum Rate: %.0f per %d ms", updated_min, memory_sampling_period_ms_),
        updated_min);
  }
}

void BasicPageFaultsTrack::DoDraw(PrimitiveAssembler& primitive_assembler,
                                  TextRenderer& text_renderer, const DrawContext& draw_context) {
  LineGraphTrack<kBasicPageFaultsTrackDimension>::DoDraw(primitive_assembler, text_renderer,
                                                         draw_context);

  if (draw_context.picking_mode != PickingMode::kNone || IsCollapsed()) return;
  AnnotationTrack::DrawAnnotation(primitive_assembler, text_renderer, layout_, indentation_level_,
                                  GlCanvas::kZValueTrackText);
}

void BasicPageFaultsTrack::DrawSingleSeriesEntry(
    PrimitiveAssembler& primitive_assembler, uint64_t start_tick, uint64_t end_tick,
    const std::array<float, kBasicPageFaultsTrackDimension>& prev_normalized_values,
    const std::array<float, kBasicPageFaultsTrackDimension>& curr_normalized_values, float z,
    bool is_last) {
  LineGraphTrack<kBasicPageFaultsTrackDimension>::DrawSingleSeriesEntry(
      primitive_assembler, start_tick, end_tick, prev_normalized_values, curr_normalized_values, z,
      is_last);

  if (!index_of_series_to_highlight_.has_value()) return;
  if (prev_normalized_values[index_of_series_to_highlight_.value()] == 0) return;

  const Color highlighting_color(231, 68, 53, 100);
  float x0 = timeline_info_->GetWorldFromTick(start_tick);
  float width = timeline_info_->GetWorldFromTick(end_tick) - x0;
  float content_height = GetGraphContentHeight();
  float y0 = GetGraphContentBottomY() - GetGraphContentHeight();
  primitive_assembler.AddShadedBox(Vec2(x0, y0), Vec2(width, content_height), z,
                                   highlighting_color);
}

bool BasicPageFaultsTrack::IsCollapsed() const {
  return LineGraphTrack<kBasicPageFaultsTrackDimension>::IsCollapsed() ||
         GetParent()->IsCollapsed();
}

float BasicPageFaultsTrack::GetAnnotatedTrackContentHeight() const {
  return GetGraphContentHeight();
}

}  // namespace orbit_gl
