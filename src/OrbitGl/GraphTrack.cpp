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

namespace {
constexpr const float kBoxHeightMultiplier = 1.5f;
}  // namespace

template <size_t Dimension>
GraphTrack<Dimension>::GraphTrack(CaptureViewElement* parent,
                                  const orbit_gl::TimelineInfoInterface* timeline_info,
                                  orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                  std::array<std::string, Dimension> series_names,
                                  uint8_t series_value_decimal_digits,
                                  std::string series_value_units,
                                  const orbit_client_data::CaptureData* capture_data)
    : Track(parent, timeline_info, viewport, layout, capture_data),
      series_{series_names, series_value_decimal_digits, std::move(series_value_units)} {}

template <size_t Dimension>
float GraphTrack<Dimension>::GetHeight() const {
  // Top content margin is counted twice because it it inserted before and after the legend
  float height = layout_->GetTrackTabHeight() + layout_->GetTrackContentTopMargin() * 2 +
                 GetLegendHeight() + GetGraphContentHeight() +
                 layout_->GetTrackContentBottomMargin();
  return height;
}

template <size_t Dimension>
float GraphTrack<Dimension>::GetLegendHeight() const {
  if (IsCollapsed() || Dimension <= 1) {
    return 0;
  }
  // This is a bit of an arbitrary choice - I don't want to introduce an additional layout
  // parameter just for the legend size, but it should be smaller than all regular textboxes
  // across Orbit.
  return layout_->GetTextBoxHeight() / 2.f;
}

template <size_t Dimension>
void GraphTrack<Dimension>::DoDraw(Batcher& batcher, TextRenderer& text_renderer,
                                   const DrawContext& draw_context) {
  Track::DoDraw(batcher, text_renderer, draw_context);
  if (IsEmpty() || IsCollapsed()) return;

  // Draw label
  const std::array<double, Dimension>& values =
      series_.GetPreviousOrFirstEntry(draw_context.current_mouse_time_ns);
  uint64_t first_time = series_.StartTimeInNs();
  uint64_t label_time = std::max(draw_context.current_mouse_time_ns, first_time);
  float point_x = timeline_info_->GetWorldFromTick(label_time);
  float point_y = GetLabelYFromValues(values);
  std::string text = GetLabelTextFromValues(values);
  const Color kBlack(0, 0, 0, 255);
  const Color kTransparentWhite(255, 255, 255, 180);
  DrawLabel(batcher, text_renderer, Vec2(point_x, point_y), text, kBlack, kTransparentWhite);

  if (Dimension == 1) return;

  // Draw legends
  const Color kWhite(255, 255, 255, 255);
  DrawLegend(batcher, text_renderer, series_.GetSeriesNames(), kWhite);
}

template <size_t Dimension>
void GraphTrack<Dimension>::DoUpdatePrimitives(Batcher& batcher, TextRenderer& text_renderer,
                                               uint64_t min_tick, uint64_t max_tick,
                                               PickingMode picking_mode) {
  ORBIT_SCOPE_WITH_COLOR("GraphTrack<Dimension>::DoUpdatePrimitives", kOrbitColorBlueGrey);
  Track::DoUpdatePrimitives(batcher, text_renderer, min_tick, max_tick, picking_mode);

  float track_z = GlCanvas::kZValueTrack;
  float graph_z = GlCanvas::kZValueEventBar;

  float content_height = GetGraphContentHeight();
  Vec2 content_pos = GetPos();
  content_pos[1] += layout_->GetTrackTabHeight();
  Box box(content_pos, Vec2(GetWidth(), content_height + GetLegendHeight()), track_z);
  batcher.AddBox(box, GetTrackBackgroundColor(), shared_from_this());

  const bool picking = picking_mode != PickingMode::kNone;
  if (picking) return;
  double time_range = static_cast<double>(max_tick - min_tick);
  if (series_.GetTimeToSeriesValuesSize() < 2 || time_range == 0) return;
  DrawSeries(batcher, min_tick, max_tick, graph_z);
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

  uint32_t font_size = layout_->CalculateZoomedFontSize();
  return (font_size * (10 - capped_indentation_level)) / 10;
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawLabel(Batcher& batcher, TextRenderer& text_renderer,
                                      Vec2 target_pos, const std::string& text,
                                      const Color& text_color, const Color& font_color) {
  uint32_t font_size = GetLegendFontSize(indentation_level_);
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
  bool arrow_is_left_directed = target_pos[0] < arrow_box_size[0] + arrow_width;
  Vec2 text_box_position(
      target_pos[0] + (arrow_is_left_directed ? arrow_width + kTextLeftMargin
                                              : -arrow_width - kTextRightMargin - text_box_size[0]),
      target_pos[1] - text_box_size[1] / 2.f);

  float label_z = GlCanvas::kZValueTrackLabel;
  text_renderer.AddText(text.c_str(), text_box_position[0], text_box_position[1], label_z,
                        {font_size, text_color, text_box_size[0]});

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
                                       const std::array<std::string, Dimension>& series_names,
                                       const Color& legend_text_color) {
  const float kSpaceBetweenLegendSymbolAndText = layout_->GetGenericFixedSpacerWidth();
  const float kSpaceBetweenLegendEntries = layout_->GetGenericFixedSpacerWidth() * 2;
  const float legend_symbol_height = GetLegendHeight();
  const float legend_symbol_width = legend_symbol_height;
  float x0 = GetPos()[0] + layout_->GetRightMargin();
  const float y0 = GetPos()[1] + layout_->GetTrackTabHeight() + layout_->GetTrackContentTopMargin();
  uint32_t font_size = GetLegendFontSize(indentation_level_);
  const Color kFullyTransparent(255, 255, 255, 0);

  float text_z = GlCanvas::kZValueTrackText;
  for (size_t i = 0; i < Dimension; ++i) {
    batcher.AddShadedBox(Vec2(x0, y0), Vec2(legend_symbol_width, legend_symbol_height), text_z,
                         GetColor(i));
    x0 += legend_symbol_width + kSpaceBetweenLegendSymbolAndText;

    const float legend_text_width =
        text_renderer.GetStringWidth(series_names[i].c_str(), font_size);
    const Vec2 legend_text_box_size(legend_text_width, layout_->GetTextBoxHeight());

    TextRenderer::TextFormatting formatting{font_size, legend_text_color, legend_text_box_size[0]};
    formatting.valign = TextRenderer::VAlign::Middle;

    text_renderer.AddText(series_names[i].c_str(), x0, y0 + legend_symbol_height / 2.f, text_z,
                          formatting);
    auto user_data = std::make_unique<PickingUserData>(
        nullptr, [this, i](PickingId /*id*/) { return GetLegendTooltips(i); });
    batcher.AddShadedBox(Vec2(x0, y0), legend_text_box_size, text_z, kFullyTransparent,
                         std::move(user_data));

    x0 += legend_text_width + kSpaceBetweenLegendEntries;
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawSeries(Batcher& batcher, uint64_t min_tick, uint64_t max_tick,
                                       float z) {
  auto entries = series_.GetEntriesAffectedByTimeRange(min_tick, max_tick);
  if (entries.empty()) return;

  double min = GetGraphMinValue();
  double inverse_value_range = GetInverseOfGraphValueRange();
  auto current_it = entries.begin();
  auto last_it = std::prev(entries.end());

  // We skip the last element because we can't calculate time passed between last element
  // and the next one.
  while (current_it != last_it) {
    std::array<double, Dimension> cumulative_values{current_it->second};
    std::partial_sum(cumulative_values.begin(), cumulative_values.end(), cumulative_values.begin());
    // For the stacked graph, computing y positions from the normalized values results in some
    // floating error. Event if the sum of values is fixed, the top of the stacked graph may not be
    // flat. To address this problem, we compute y positions from the normalized cumulative values.
    std::array<float, Dimension> normalized_cumulative_values;
    std::transform(cumulative_values.begin(), cumulative_values.end(),
                   normalized_cumulative_values.begin(), [min, inverse_value_range](double value) {
                     return static_cast<float>((value - min) * inverse_value_range);
                   });

    uint64_t current_time = std::max(current_it->first, min_tick);
    auto next_it = std::next(current_it);
    uint64_t next_time = std::min(next_it->first, max_tick);
    DrawSingleSeriesEntry(batcher, current_time, next_time, normalized_cumulative_values, z);
    current_it = next_it;
  }
}

template <size_t Dimension>
void GraphTrack<Dimension>::DrawSingleSeriesEntry(
    Batcher& batcher, uint64_t start_tick, uint64_t end_tick,
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
    batcher.AddShadedBox(Vec2(x0, y0), Vec2(width, height), z, GetColor(i));
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
