// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GraphTrack.h"

#include <GteVector.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <numeric>

#include "Geometry.h"
#include "GlCanvas.h"
#include "OrbitBase/ThreadConstants.h"
#include "TextRenderer.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"
#include "Viewport.h"

template <size_t Dimension>
GraphTrack<Dimension>::GraphTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                                  orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                  const std::string& name,
                                  std::array<std::string, Dimension> series_names,
                                  const orbit_client_model::CaptureData* capture_data)
    : Track(parent, time_graph, viewport, layout, capture_data),
      series_(MultivariateTimeSeries<Dimension>(series_names)) {
  SetName(name);
  SetLabel(name);
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetHeight() const {
  float scale_factor = IsCollapsed() ? 1 : Dimension;
  float height = layout_->GetTrackTabHeight() + GetLegendHeight() +
                 layout_->GetTextBoxHeight() * scale_factor +
                 layout_->GetSpaceBetweenTracksAndThread() + layout_->GetEventTrackHeightFromTid() +
                 layout_->GetTrackBottomMargin();
  return height;
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetLegendHeight() const {
  bool has_legend = !IsCollapsed() && Dimension > 1;
  float legend_height = has_legend ? layout_->GetTextBoxHeight() : 0;
  return legend_height;
}

template <size_t Dimension>
void GraphTrack<Dimension>::Draw(Batcher& batcher, TextRenderer& text_renderer,
                                 const DrawContext& draw_context) {
  Track::Draw(batcher, text_renderer, draw_context);
  if (IsEmpty() || IsCollapsed()) return;

  // Draw label
  const std::array<double, Dimension>& values =
      series_.GetPreviousOrFirstEntry(draw_context.current_mouse_time_ns)->second;
  uint64_t first_time = series_.StartTimeInNs();
  uint64_t label_time = std::max(draw_context.current_mouse_time_ns, first_time);
  float point_x = time_graph_->GetWorldFromTick(label_time);
  float point_y = GetLabelYFromValues(values);
  std::string text = GetLabelTextFromValues(values);
  const Color kBlack(0, 0, 0, 255);
  const Color kTransparentWhite(255, 255, 255, 180);
  DrawLabel(batcher, text_renderer, draw_context, Vec2(point_x, point_y), text, kBlack,
            kTransparentWhite);

  if (Dimension == 1) return;

  // Draw legends
  const Color kWhite(255, 255, 255, 255);
  DrawLegend(batcher, text_renderer, draw_context, series_.GetSeriesNames(), kWhite);
}

template <size_t Dimension>
void GraphTrack<Dimension>::UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                             PickingMode picking_mode, float z_offset) {
  pos_[0] = viewport_->GetWorldTopLeft()[0];

  float track_z = GlCanvas::kZValueTrack + z_offset;
  float graph_z = GlCanvas::kZValueEventBar + z_offset;

  float content_height = GetGraphContentHeight();
  Vec2 content_pos = pos_;
  content_pos[1] -= layout_->GetTrackTabHeight();
  Box box(content_pos, Vec2(size_[0], -content_height - GetLegendHeight()), track_z);
  batcher->AddBox(box, GetTrackBackgroundColor(), shared_from_this());

  const bool picking = picking_mode != PickingMode::kNone;
  if (picking) return;
  double time_range = static_cast<double>(max_tick - min_tick);
  if (series_.GetTimeToSeriesValues().size() < 2 || time_range == 0) return;
  DrawSeries(batcher, min_tick, max_tick, graph_z);
}

template <size_t Dimension>
Color GraphTrack<Dimension>::GetColor(size_t index) const {
  if (series_colors_.has_value()) return series_colors_.value()[index];
  return TimeGraph::GetColor(index);
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetLabelYFromValues(
    const std::array<double, Dimension>& /*values*/) const {
  return GetGraphContentBaseY() + GetGraphContentHeight() / 2.f;
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
        value_decimal_digits.has_value()
            ? absl::StrFormat("%.*f", value_decimal_digits.value(), values[i])
            : std::to_string(values[i]);
    absl::StrAppend(&text, delimiter, formatted_name, formatted_value, value_unit);
    delimiter = "\n";
  }
  return text;
}

template <size_t Dimension>
uint32_t GraphTrack<Dimension>::GetLegendFontSize(uint32_t indentation_level) const {
  constexpr uint32_t kMinIndentationLevel = 1;
  int capped_indentation_level = std::min(indentation_level, kMinIndentationLevel);

  uint32_t font_size = layout_->CalculateZoomedFontSize();
  return (font_size * (10 - capped_indentation_level)) / 10;
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawLabel(Batcher& batcher, TextRenderer& text_renderer,
                                      const DrawContext& draw_context, Vec2 target_pos,
                                      const std::string& text, const Color& text_color,
                                      const Color& font_color) {
  uint32_t font_size = GetLegendFontSize(draw_context.indentation_level);
  const float kTextLeftMargin = 2.f;
  const float kTextRightMargin = kTextLeftMargin;
  const float kTextTopMargin = layout_->GetTextOffset();
  const float kTextBottomMargin = kTextTopMargin;
  const float kSpaceBetweenLines = layout_->GetTextOffset();

  std::vector<std::string> lines = absl::StrSplit(text, '\n');
  float text_width = 0;
  for (const std::string& line : lines) {
    text_width = std::max(text_width, text_renderer.GetStringWidth(line.c_str(), font_size));
  }
  float single_line_height = text_renderer.GetStringHeight(text.c_str(), font_size);
  float text_height = single_line_height * lines.size() + kSpaceBetweenLines * (lines.size() - 1);
  Vec2 text_box_size(text_width, text_height);

  float arrow_width = text_box_size[1] / 2.f;
  Vec2 arrow_box_size(text_box_size[0] + kTextLeftMargin + kTextRightMargin,
                      text_box_size[1] + kTextTopMargin + kTextBottomMargin);
  bool arrow_is_left_directed =
      target_pos[0] < viewport_->GetWorldTopLeft()[0] + arrow_box_size[0] + arrow_width;
  Vec2 text_box_position(
      target_pos[0] + (arrow_is_left_directed ? arrow_width + kTextLeftMargin
                                              : -arrow_width - kTextRightMargin - text_box_size[0]),
      target_pos[1] - text_box_size[1] / 2.f);

  float label_z = GlCanvas::kZValueTrackLabel + draw_context.z_offset;
  float bottom_y_of_top_line = text_box_position[1] + text_box_size[1] - single_line_height;
  text_renderer.AddText(text.c_str(), text_box_position[0], bottom_y_of_top_line, label_z,
                        text_color, font_size, text_box_size[0]);

  Vec2 arrow_box_position(text_box_position[0] - kTextLeftMargin,
                          text_box_position[1] - kTextBottomMargin);
  Box arrow_text_box(arrow_box_position, arrow_box_size, label_z);
  Vec3 arrow_extra_point(target_pos[0], target_pos[1], label_z);

  batcher.AddBox(arrow_text_box, font_color);
  if (arrow_is_left_directed) {
    batcher.AddTriangle(
        Triangle(arrow_text_box.vertices[0], arrow_text_box.vertices[1], arrow_extra_point),
        font_color);
  } else {
    batcher.AddTriangle(
        Triangle(arrow_text_box.vertices[2], arrow_text_box.vertices[3], arrow_extra_point),
        font_color);
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawLegend(Batcher& batcher, TextRenderer& text_renderer,
                                       const DrawContext& draw_context,
                                       const std::array<std::string, Dimension>& series_names,
                                       const Color& legend_text_color) {
  const float kSpaceBetweenLegendSymbolAndText = layout_->GetGenericFixedSpacerWidth();
  const float kSpaceBetweenLegendEntries = layout_->GetGenericFixedSpacerWidth() * 2;
  float legend_symbol_height = GetLegendHeight() / 2.f;
  float legend_symbol_width = legend_symbol_height;
  float x0 = pos_[0] + layout_->GetRightMargin();
  float y0 = pos_[1] - layout_->GetTrackTabHeight() - layout_->GetTextBoxHeight() / 2.f;
  uint32_t font_size = GetLegendFontSize(draw_context.indentation_level);
  const Color kFullyTransparent(255, 255, 255, 0);

  float text_z = GlCanvas::kZValueTrackText + draw_context.z_offset;
  for (size_t i = 0; i < Dimension; ++i) {
    batcher.AddShadedBox(Vec2(x0, y0 - legend_symbol_height / 2.f),
                         Vec2(legend_symbol_width, legend_symbol_height), text_z, GetColor(i));
    x0 += legend_symbol_width + kSpaceBetweenLegendSymbolAndText;

    float legend_text_width = text_renderer.GetStringWidth(series_names[i].c_str(), font_size);
    Vec2 legend_text_box_size(legend_text_width, layout_->GetTextBoxHeight());
    Vec2 legend_text_box_position(x0, y0 - layout_->GetTextBoxHeight() / 2.f);
    text_renderer.AddText(series_names[i].c_str(), legend_text_box_position[0],
                          legend_text_box_position[1] + layout_->GetTextOffset(), text_z,
                          legend_text_color, font_size, legend_text_box_size[0]);
    x0 += legend_text_width + kSpaceBetweenLegendEntries;

    auto user_data = std::make_unique<PickingUserData>(
        nullptr, [this, i](PickingId /*id*/) { return GetLegendTooltips(i); });
    batcher.AddShadedBox(legend_text_box_position, legend_text_box_size, text_z, kFullyTransparent,
                         std::move(user_data));
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawSeries(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                       float z) {
  auto entries_affected_range_result = series_.GetEntriesAffectedByTimeRange(min_tick, max_tick);
  if (!entries_affected_range_result.has_value()) return;

  typename MultivariateTimeSeries<Dimension>::Range& entries{entries_affected_range_result.value()};

  double min = GetGraphMinValue();
  double inverse_value_range = GetInverseOfGraphValueRange();

  auto current_iterator = entries.start_inclusive;
  while (current_iterator != entries.end_inclusive) {
    std::array<double, Dimension> cumulative_values{current_iterator->second};
    std::partial_sum(cumulative_values.begin(), cumulative_values.end(), cumulative_values.begin());
    // For the stacked graph, computing y positions from the normalized values results in some
    // floating error. Event if the sum of values is fixed, the top of the stacked graph may not be
    // flat. To address this problem, we compute y positions from the normalized cumulative values.
    std::array<float, Dimension> normalized_cumulative_values;
    std::transform(cumulative_values.begin(), cumulative_values.end(),
                   normalized_cumulative_values.begin(), [min, inverse_value_range](double value) {
                     return static_cast<float>((value - min) * inverse_value_range);
                   });

    uint64_t current_time = std::max(current_iterator->first, min_tick);
    auto next_iterator = std::next(current_iterator);
    uint64_t next_time = std::min(next_iterator->first, max_tick);
    DrawSingleSeriesEntry(batcher, current_time, next_time, normalized_cumulative_values, z);
    current_iterator = next_iterator;
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawSingleSeriesEntry(
    Batcher* batcher, uint64_t start_tick, uint64_t end_tick,
    const std::array<float, Dimension>& normalized_cumulative_values, float z) {
  float x0 = time_graph_->GetWorldFromTick(start_tick);
  float width = time_graph_->GetWorldFromTick(end_tick) - x0;
  float content_height = GetGraphContentHeight();
  float base_y = GetGraphContentBaseY();
  float y0 = base_y;
  for (size_t i = 0; i < Dimension; ++i) {
    float height = base_y + normalized_cumulative_values[i] * content_height - y0;
    batcher->AddShadedBox(Vec2(x0, y0), Vec2(width, height), z, GetColor(i));
    y0 += height;
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
