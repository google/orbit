// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LineGraphTrack.h"

#include <GteVector.h>

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "Geometry.h"

namespace orbit_gl {

template <size_t Dimension>
[[nodiscard]] static std::array<float, Dimension> GetNormalizedValues(
    const std::array<double, Dimension>& values, double min, double inverse_value_range) {
  std::array<float, Dimension> normalized_values;
  std::transform(values.begin(), values.end(), normalized_values.begin(),
                 [min, inverse_value_range](double value) {
                   return static_cast<float>((value - min) * inverse_value_range);
                 });
  return normalized_values;
}

template <size_t Dimension>
float LineGraphTrack<Dimension>::GetLabelYFromValues(
    const std::array<double, Dimension>& values) const {
  float content_height = this->GetGraphContentHeight();
  float base_y = this->GetGraphContentBottomY();
  double min = this->GetGraphMinValue();
  double inverse_value_range = this->GetInverseOfGraphValueRange();
  std::array<float, Dimension> normalized_values =
      GetNormalizedValues(values, min, inverse_value_range);
  // The label will point to the only value.
  if (Dimension == 1) return base_y - normalized_values[0] * content_height;

  // The label will be centred on the track.
  return base_y - content_height / 2.0f;
}

template <size_t Dimension>
void LineGraphTrack<Dimension>::DrawSeries(PrimitiveAssembler& primitive_assembler,
                                           uint64_t min_tick, uint64_t max_tick, float z) {
  auto entries = this->series_.GetEntriesAffectedByTimeRange(min_tick, max_tick);
  if (entries.empty()) return;

  double min = this->GetGraphMinValue();
  double inverse_value_range = this->GetInverseOfGraphValueRange();

  auto curr_iterator = entries.begin();
  auto last_iterator = std::prev(entries.end());
  std::array<float, Dimension> prev_normalized_values =
      GetNormalizedValues(curr_iterator->second, min, inverse_value_range);

  bool is_first = true;
  GraphTrackDataAggregator<Dimension> aggr(this->aggregation_mode_);

  const uint32_t resolution_in_pixels =
      this->GetViewport()->WorldToScreen({this->GetWidth(), 0})[0];
  uint64_t next_pixel_start_ns = orbit_client_data::GetNextPixelBoundaryTimeNs(
      min_tick, resolution_in_pixels, min_tick, max_tick);

  while (curr_iterator != last_iterator) {
    uint64_t prev_time = curr_iterator->first;
    curr_iterator = std::next(curr_iterator);

    uint64_t curr_time = curr_iterator->first;
    std::array<float, Dimension> curr_normalized_values =
        GetNormalizedValues(curr_iterator->second, min, inverse_value_range);

    if (is_first) {
      aggr.StartNewEntry(prev_time, curr_time, curr_normalized_values);
      is_first = false;
    } else {
      // If the size of the current entry is less than a pixel
      if (aggr.GetEntry().end_tick < next_pixel_start_ns) {
        // Add the current data to accumulated_entry
        aggr.AppendData(prev_time, curr_time, curr_normalized_values);
      } else {
        // Otherwise, draw the accumulated_entry and start accumulating a new one
        DrawSingleSeriesEntry(primitive_assembler, aggr.GetEntry().start_tick,
                              aggr.GetEntry().end_tick, prev_normalized_values,
                              aggr.GetEntry().values, z, false);

        next_pixel_start_ns = orbit_client_data::GetNextPixelBoundaryTimeNs(
            aggr.GetEntry().end_tick, resolution_in_pixels, min_tick, max_tick);

        prev_normalized_values = aggr.GetEntry().values;
        aggr.StartNewEntry(prev_time, curr_time, curr_normalized_values);
      }
    }
  }

  bool is_accumulated_entry_last = aggr.GetEntry().end_tick >= max_tick;
  // Draw the leftover entry
  DrawSingleSeriesEntry(primitive_assembler, aggr.GetEntry().start_tick, aggr.GetEntry().end_tick,
                        prev_normalized_values, aggr.GetEntry().values, z,
                        is_accumulated_entry_last);

  // If there was not enough data to reach the end tick, draw an entry until the
  // end.
  if (!is_accumulated_entry_last) {
    DrawSingleSeriesEntry(primitive_assembler, aggr.GetEntry().end_tick, max_tick,
                          aggr.GetEntry().values, aggr.GetEntry().values, z, true);
  }
}

static void DrawSquareDot(PrimitiveAssembler& primitive_assembler, Vec2 center, float radius,
                          float z, const Color& color) {
  Vec2 position(center[0] - radius, center[1] - radius);
  Vec2 size(2 * radius, 2 * radius);
  primitive_assembler.AddBox(MakeBox(position, size), z, color);
}

template <size_t Dimension>
void LineGraphTrack<Dimension>::DrawSingleSeriesEntry(
    PrimitiveAssembler& primitive_assembler, uint64_t start_tick, uint64_t end_tick,
    const std::array<float, Dimension>& prev_normalized_values,
    const std::array<float, Dimension>& curr_normalized_values, float z, bool is_last) {
  constexpr float kDotRadius = 2.f;
  float x0 = this->timeline_info_->GetWorldFromTick(start_tick);
  float x1 = this->timeline_info_->GetWorldFromTick(end_tick);
  float content_height = this->GetGraphContentHeight();
  float base_y = this->GetGraphContentBottomY();

  for (size_t i = Dimension; i-- > 0;) {
    float y0 = base_y - prev_normalized_values[i] * content_height;
    DrawSquareDot(primitive_assembler, Vec2(x0, y0), kDotRadius, z, this->GetColor(i));
    primitive_assembler.AddLine(Vec2(x0, y0), Vec2(x1, y0), z, this->GetColor(i));
    if (!is_last) {
      float y1 = base_y - curr_normalized_values[i] * content_height;
      primitive_assembler.AddLine(Vec2(x1, y0), Vec2(x1, y1), z, this->GetColor(i));
    }
  }
}

template class LineGraphTrack<1>;
template class LineGraphTrack<2>;
template class LineGraphTrack<3>;
template class LineGraphTrack<4>;

}  // namespace orbit_gl
