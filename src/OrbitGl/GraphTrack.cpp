// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GraphTrack.h"

#include <GteVector.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <numeric>
#include <string_view>
#include <utility>
#include <vector>

#include "ApiInterface/Orbit.h"
#include "ClientData/FastRenderingUtils.h"
#include "OrbitGl/BatcherInterface.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/GraphTrackDataAggregator.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraph.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TrackHeader.h"
#include "OrbitGl/Viewport.h"

namespace {
constexpr const float kBoxHeightMultiplier = 1.5f;
}  // namespace

using orbit_gl::PickingUserData;
using orbit_gl::PrimitiveAssembler;
using orbit_gl::TextRenderer;

template <size_t Dimension>
GraphTrack<Dimension>::GraphTrack(CaptureViewElement* parent,
                                  const orbit_gl::TimelineInfoInterface* timeline_info,
                                  orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                  std::array<std::string, Dimension> series_names,
                                  uint8_t series_value_decimal_digits,
                                  std::string series_value_units,
                                  const orbit_client_data::ModuleManager* module_manager,
                                  const orbit_client_data::CaptureData* capture_data)
    : Track(parent, timeline_info, viewport, layout, module_manager, capture_data),
      series_{series_names, series_value_decimal_digits, std::move(series_value_units)} {}

template <size_t Dimension>
bool GraphTrack<Dimension>::HasLegend() const {
  return !IsCollapsed() && Dimension > 1;
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetHeight() const {
  // Top content margin is counted twice when there are legends because it is inserted above and
  // below the legend.
  float height_above_content =
      GetLegendHeight() + (HasLegend() ? 2.f : 1.f) * layout_->GetTrackContentTopMargin();

  return layout_->GetTrackTabHeight() + height_above_content + GetGraphContentHeight() +
         layout_->GetTrackContentBottomMargin();
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetLegendHeight() const {
  if (!HasLegend()) return 0;

  // Legend size should be smaller than all regular textboxes across Orbit.
  const float legend_height = layout_->GetTextBoxHeight() / 2.f;
  return legend_height;
}

template <size_t Dimension>
void GraphTrack<Dimension>::DoDraw(PrimitiveAssembler& primitive_assembler,
                                   TextRenderer& text_renderer, const DrawContext& draw_context) {
  Track::DoDraw(primitive_assembler, text_renderer, draw_context);
  if (IsEmpty() || IsCollapsed()) return;

  DrawMouseLabel(primitive_assembler, text_renderer, draw_context);

  if (!HasLegend()) return;

  // Draw legends
  const Color white(255, 255, 255, 255);
  DrawLegend(primitive_assembler, text_renderer, series_.GetSeriesNames(), white);
}

template <size_t Dimension>
void GraphTrack<Dimension>::DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler,
                                               TextRenderer& text_renderer, uint64_t min_tick,
                                               uint64_t max_tick, PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("GraphTrack<Dimension>::DoUpdatePrimitives", kOrbitColorBlueGrey);
  Track::DoUpdatePrimitives(primitive_assembler, text_renderer, min_tick, max_tick, picking_mode);

  float track_z = GlCanvas::kZValueTrack;
  float graph_z = GlCanvas::kZValueEventBar;

  float content_height = GetGraphContentHeight();
  Vec2 content_pos = GetPos();
  content_pos[1] += layout_->GetTrackTabHeight();
  Quad box = MakeBox(content_pos, Vec2(GetWidth(), content_height + GetLegendHeight()));
  primitive_assembler.AddBox(box, track_z, GetTrackBackgroundColor(), shared_from_this());

  const bool picking = picking_mode != PickingMode::kNone;
  if (picking) return;
  auto time_range = static_cast<double>(max_tick - min_tick);
  if (series_.GetTimeToSeriesValuesSize() < 2 || time_range == 0) return;
  DrawSeries(primitive_assembler, min_tick, max_tick, graph_z);
}

template <size_t Dimension>
Color GraphTrack<Dimension>::GetColor(size_t index) const {
  if (series_colors_.has_value()) return series_colors_.value()[index];
  return TimeGraph::GetColor(index);
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetGraphContentHeight() const {
  float result = layout_->GetTextBoxHeight() * kBoxHeightMultiplier;
  if (!IsCollapsed()) {
    result *= Dimension;
  }
  return result;
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetLabelYFromValues(
    const std::array<double, Dimension>& /*values*/) const {
  return GetGraphContentBottomY() - GetGraphContentHeight() / 2.f;
}

template <size_t Dimension>
std::string GraphTrack<Dimension>::GetLabelTextFromValues(
    const std::array<double, Dimension>& values) const {
  const std::array<std::string, Dimension>& series_names = series_.GetSeriesNames();
  std::optional<uint8_t> value_decimal_digits = series_.GetValueDecimalDigits();
  std::string value_unit = series_.GetValueUnit();
  std::string text;
  std::string_view delimiter = "";
  for (int i = Dimension - 1; i >= 0; i--) {
    std::string formatted_name =
        series_names[i].empty() ? "" : absl::StrFormat("%s: ", series_names[i]);
    std::string formatted_value =
        absl::StrFormat("%.*f", value_decimal_digits.value_or(6), values[i]);
    absl::StrAppend(&text, delimiter, formatted_name, formatted_value, value_unit);
    delimiter = "\n";
  }
  return text;
}

template <size_t Dimension>
uint32_t GraphTrack<Dimension>::GetLegendFontSize(uint32_t indentation_level) const {
  constexpr uint32_t kMinIndentationLevel = 1;
  int capped_indentation_level = std::max(indentation_level, kMinIndentationLevel);

  uint32_t font_size = layout_->GetFontSize();
  return (font_size * (10 - capped_indentation_level)) / 10;
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawMouseLabel(PrimitiveAssembler& primitive_assembler,
                                           TextRenderer& text_renderer,
                                           const DrawContext& draw_context) {
  if (!draw_context.current_mouse_tick.has_value()) return;

  const float text_left_margin = 2.f;
  const float text_right_margin = text_left_margin;
  const float text_top_margin = layout_->GetTextOffset();
  const float text_bottom_margin = text_top_margin;
  const Color black(0, 0, 0, 255);
  const Color transparent_white(255, 255, 255, 180);

  uint64_t current_mouse_time_ns = draw_context.current_mouse_tick.value();
  const std::array<double, Dimension>& values =
      series_.GetPreviousOrFirstEntry(current_mouse_time_ns);
  uint64_t first_time = series_.StartTimeInNs();
  uint64_t label_time = std::max(current_mouse_time_ns, first_time);
  Vec2 target_point_pos{timeline_info_->GetWorldFromTick(label_time), GetLabelYFromValues(values)};
  std::string text = GetLabelTextFromValues(values);
  uint32_t font_size = GetLegendFontSize(indentation_level_);

  std::vector<std::string> lines = absl::StrSplit(text, '\n');
  float text_width = 0;
  for (const std::string& line : lines) {
    text_width = std::max(text_width, text_renderer.GetStringWidth(line.c_str(), font_size));
  }
  float text_height = text_renderer.GetStringHeight(text.c_str(), font_size);
  Vec2 text_box_size(text_width, text_height);

  float arrow_width = text_box_size[1] / 2.f;
  Vec2 arrow_box_size(text_box_size[0] + text_left_margin + text_right_margin,
                      text_box_size[1] + text_top_margin + text_bottom_margin);
  bool arrow_is_left_directed = target_point_pos[0] < arrow_box_size[0] + arrow_width;
  Vec2 text_box_position(
      target_point_pos[0] + (arrow_is_left_directed
                                 ? arrow_width + text_left_margin
                                 : -arrow_width - text_right_margin - text_box_size[0]),
      target_point_pos[1] - text_box_size[1] / 2.f);

  float label_z = GlCanvas::kZValueTrackLabel;
  text_renderer.AddText(text.c_str(), text_box_position[0], text_box_position[1], label_z,
                        {font_size, black, text_box_size[0]});

  Vec2 arrow_box_position(text_box_position[0] - text_left_margin,
                          text_box_position[1] - text_bottom_margin);
  Quad arrow_text_box = MakeBox(arrow_box_position, arrow_box_size);
  Vec2 arrow_extra_point(target_point_pos[0], target_point_pos[1]);

  primitive_assembler.AddBox(arrow_text_box, label_z, transparent_white);
  if (arrow_is_left_directed) {
    primitive_assembler.AddTriangle(
        Triangle(arrow_text_box.vertices[0], arrow_text_box.vertices[1], arrow_extra_point),
        label_z, transparent_white);
  } else {
    primitive_assembler.AddTriangle(
        Triangle(arrow_text_box.vertices[2], arrow_text_box.vertices[3], arrow_extra_point),
        label_z, transparent_white);
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawLegend(PrimitiveAssembler& primitive_assembler,
                                       TextRenderer& text_renderer,
                                       const std::array<std::string, Dimension>& series_names,
                                       const Color& legend_text_color) {
  const float space_between_legend_symbol_and_text = layout_->GetGenericFixedSpacerWidth();
  const float space_between_legend_entries = layout_->GetGenericFixedSpacerWidth() * 2;
  const float legend_symbol_height = GetLegendHeight();
  const float legend_symbol_width = legend_symbol_height;
  float x0 = GetPos()[0] + layout_->GetRightMargin();
  const float y0 =
      header_->GetPos()[1] + header_->GetHeight() + layout_->GetTrackContentTopMargin();
  uint32_t font_size = GetLegendFontSize(indentation_level_);
  const Color fully_transparent(255, 255, 255, 0);

  float text_z = GlCanvas::kZValueTrackText;
  for (size_t i = 0; i < Dimension; ++i) {
    primitive_assembler.AddShadedBox(Vec2(x0, y0), Vec2(legend_symbol_width, legend_symbol_height),
                                     text_z, GetColor(i));
    x0 += legend_symbol_width + space_between_legend_symbol_and_text;

    const float legend_text_width =
        text_renderer.GetStringWidth(series_names[i].c_str(), font_size);
    const Vec2 legend_text_box_size(legend_text_width, layout_->GetTextBoxHeight());

    TextRenderer::TextFormatting formatting{font_size, legend_text_color, legend_text_box_size[0]};
    formatting.valign = TextRenderer::VAlign::Middle;

    text_renderer.AddText(series_names[i].c_str(), x0, y0 + legend_symbol_height / 2.f, text_z,
                          formatting);
    auto user_data = std::make_unique<PickingUserData>(
        nullptr, [this, i](PickingId /*id*/) { return GetLegendTooltips(i); });
    primitive_assembler.AddShadedBox(Vec2(x0, y0), legend_text_box_size, text_z, fully_transparent,
                                     std::move(user_data));

    x0 += legend_text_width + space_between_legend_entries;
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawSeries(PrimitiveAssembler& primitive_assembler, uint64_t min_tick,
                                       uint64_t max_tick, float z) {
  auto entries = series_.GetEntriesAffectedByTimeRange(min_tick, max_tick);
  if (entries.empty()) return;

  double min = GetGraphMinValue();
  double inverse_value_range = GetInverseOfGraphValueRange();
  auto current_it = entries.begin();
  auto last_it = std::prev(entries.end());

  GraphTrackDataAggregator<Dimension> aggr;

  const uint32_t resolution_in_pixels = viewport_->WorldToScreen({GetWidth(), 0})[0];
  uint64_t next_pixel_start_ns = orbit_client_data::GetNextPixelBoundaryTimeNs(
      min_tick, resolution_in_pixels, min_tick, max_tick);

  // We skip the last element because we can't calculate time passed between last element
  // and the next one.
  while (current_it != last_it) {
    std::array<double, Dimension> cumulative_values{current_it->second};
    std::partial_sum(cumulative_values.begin(), cumulative_values.end(), cumulative_values.begin());
    // For the stacked graph, computing y positions from the normalized values results in some
    // floating error. Event if the sum of values is fixed, the top of the stacked graph may not be
    // flat. To address this problem, we compute y positions from the normalized cumulative values.
    std::array<float, Dimension> normalized_cumulative_values{};
    std::transform(cumulative_values.begin(), cumulative_values.end(),
                   normalized_cumulative_values.begin(), [min, inverse_value_range](double value) {
                     return static_cast<float>((value - min) * inverse_value_range);
                   });

    uint64_t current_time = std::max(current_it->first, min_tick);
    auto next_it = std::next(current_it);
    uint64_t next_time = std::min(next_it->first, max_tick);

    if (aggr.GetAccumulatedEntry() == nullptr) {
      aggr.SetEntry(current_time, next_time, normalized_cumulative_values);
    } else {
      // If the current data point fits into the same pixel as the entry we are currently
      // accumulating.
      if (current_time < next_pixel_start_ns) {
        // Add the current data to accumulated_entry
        aggr.MergeDataIntoEntry(current_time, next_time, normalized_cumulative_values);
      } else {
        // Otherwise, draw the accumulated_entry and start accumulating a new one
        // When drawing we only use max values - for every usage of this track
        // this is currently the best representation.
        // If we draw multiple boxes on the same pixel, the largest box would
        // overdraw the smaller ones.
        DrawSingleSeriesEntry(primitive_assembler, aggr.GetAccumulatedEntry()->start_tick,
                              aggr.GetAccumulatedEntry()->end_tick,
                              aggr.GetAccumulatedEntry()->max_vals, z);

        // Must be done before the next `SetEntry` call - we are using the end tick value of the
        // current entry to calculate the next pixel border.
        next_pixel_start_ns = orbit_client_data::GetNextPixelBoundaryTimeNs(
            aggr.GetAccumulatedEntry()->end_tick, resolution_in_pixels, min_tick, max_tick);

        aggr.SetEntry(current_time, next_time, normalized_cumulative_values);
      }
    }
    current_it = next_it;
  }

  // Draw the leftover entry
  DrawSingleSeriesEntry(primitive_assembler, aggr.GetAccumulatedEntry()->start_tick,
                        aggr.GetAccumulatedEntry()->end_tick, aggr.GetAccumulatedEntry()->max_vals,
                        z);
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawSingleSeriesEntry(
    PrimitiveAssembler& primitive_assembler, uint64_t start_tick, uint64_t end_tick,
    const std::array<float, Dimension>& normalized_cumulative_values, float z) {
  const float x0 = timeline_info_->GetWorldFromTick(start_tick);
  const float width = timeline_info_->GetWorldFromTick(end_tick) - x0;
  const float content_height = GetGraphContentHeight();
  const float base_y = GetGraphContentBottomY();
  float y0 = base_y;
  // Draw the stacked values from bottom to top
  for (size_t i = 0; i < Dimension; ++i) {
    float height = normalized_cumulative_values[i] * content_height - (base_y - y0);
    y0 -= height;
    primitive_assembler.AddShadedBox(Vec2(x0, y0), Vec2(width, height), z, GetColor(i));
  }
}

template <size_t Dimension>
uint64_t GraphTrack<Dimension>::GetMinTime() const {
  return series_.StartTimeInNs();
}
template <size_t Dimension>
uint64_t GraphTrack<Dimension>::GetMaxTime() const {
  return series_.EndTimeInNs();
}

template class GraphTrack<1>;
template class GraphTrack<2>;
template class GraphTrack<3>;
template class GraphTrack<4>;
