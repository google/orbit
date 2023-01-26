// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/LineGraphTrack.h"

#include <GteVector.h>
#include <absl/types/span.h>
#include <stddef.h>

#include <algorithm>
#include <iterator>
#include <utility>
#include <vector>

#include "ClientData/FastRenderingUtils.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/Geometry.h"
#include "OrbitGl/GraphTrackDataAggregator.h"
#include "OrbitGl/MultivariateTimeSeries.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

namespace {

[[nodiscard]] static std::vector<float> GetNormalizedValues(absl::Span<const double> values,
                                                            double min,
                                                            double inverse_value_range) {
  std::vector<float> normalized_values(values.size());
  std::transform(values.begin(), values.end(), normalized_values.begin(),
                 [min, inverse_value_range](double value) {
                   return static_cast<float>((value - min) * inverse_value_range);
                 });
  return normalized_values;
}

static void DrawSquareDot(PrimitiveAssembler& primitive_assembler, Vec2 center, float radius,
                          float z, const Color& color) {
  Vec2 position(center[0] - radius, center[1] - radius);
  Vec2 size(2 * radius, 2 * radius);
  primitive_assembler.AddBox(MakeBox(position, size), z, color);
}

}  // namespace

float LineGraphTrack::GetLabelYFromValues(absl::Span<const double> values) const {
  float content_height = GetGraphContentHeight();
  float base_y = GetGraphContentBottomY();
  double min = GetGraphMinValue();
  double inverse_value_range = GetInverseOfGraphValueRange();
  std::vector<float> normalized_values = GetNormalizedValues(values, min, inverse_value_range);
  // The label will point to the only value.
  if (series_.GetDimension() == 1) return base_y - normalized_values[0] * content_height;

  // The label will be centred on the track.
  return base_y - content_height / 2.0f;
}

void LineGraphTrack::DrawSeries(PrimitiveAssembler& primitive_assembler, uint64_t min_tick,
                                uint64_t max_tick, float z) {
  auto entries = series_.GetEntriesAffectedByTimeRange(min_tick, max_tick);
  if (entries.empty()) return;

  double min = GetGraphMinValue();
  double inverse_value_range = GetInverseOfGraphValueRange();

  auto curr_iterator = entries.begin();
  auto last_iterator = std::prev(entries.end());

  // Normalized values that were last used for drawing.
  std::vector<float> prev_drawn_values =
      GetNormalizedValues(curr_iterator->second, min, inverse_value_range);

  // Normalized values of the last entry we've iterated over.
  std::vector<float> last_entry_values = prev_drawn_values;

  GraphTrackDataAggregator aggr;

  const uint32_t resolution_in_pixels = GetViewport()->WorldToScreen({GetWidth(), 0})[0];
  uint64_t next_pixel_start_ns = orbit_client_data::GetNextPixelBoundaryTimeNs(
      min_tick, resolution_in_pixels, min_tick, max_tick);

  // Issues draws for the aggregated entry depending on `aggregation_mode_` and updates
  // `prev_normalized_values` with last drawn values.
  auto draw_aggregated = [&](const GraphTrackDataAggregator::AccumulatedEntry& accumulated_entry,
                             bool is_last) {
    // First draw the entry for the max values.
    DrawSingleSeriesEntry(primitive_assembler, accumulated_entry.start_tick,
                          accumulated_entry.end_tick, prev_drawn_values, accumulated_entry.max_vals,
                          z, is_last);
    prev_drawn_values = accumulated_entry.max_vals;

    // Draw min values if needed.
    if (aggregation_mode_ == AggregationMode::kMinMax &&
        accumulated_entry.min_vals != accumulated_entry.max_vals) {
      // Draw a single-sized entry (starts and ends at `end_tick`) that goes from max to min
      // values.
      DrawSingleSeriesEntry(primitive_assembler, accumulated_entry.end_tick,
                            accumulated_entry.end_tick, prev_drawn_values,
                            accumulated_entry.min_vals, z, is_last);
      prev_drawn_values = accumulated_entry.min_vals;
    }

    // Finally, draw the last entry. This ensures that the horizontal line from this pixel to
    // the next entry would be at the same position both zoomed in and out.
    if (last_entry_values != prev_drawn_values) {
      DrawSingleSeriesEntry(primitive_assembler, accumulated_entry.end_tick,
                            accumulated_entry.end_tick, prev_drawn_values, last_entry_values, z,
                            is_last);
      prev_drawn_values = last_entry_values;
    }
  };

  while (curr_iterator != last_iterator) {
    uint64_t prev_time = curr_iterator->first;
    curr_iterator = std::next(curr_iterator);

    uint64_t curr_time = curr_iterator->first;
    std::vector<float> curr_normalized_values =
        GetNormalizedValues(curr_iterator->second, min, inverse_value_range);

    if (aggr.GetAccumulatedEntry() == nullptr) {
      aggr.SetEntry(prev_time, curr_time, curr_normalized_values);
      last_entry_values = curr_normalized_values;
      continue;
    }

    // If the current data point fits into the same pixel as the entry we are currently
    // accumulating.
    if (curr_time < next_pixel_start_ns) {
      // Add the current data to accumulated_entry
      aggr.MergeDataIntoEntry(prev_time, curr_time, curr_normalized_values);
    } else {
      auto accumulated_entry = *aggr.GetAccumulatedEntry();
      // Otherwise, draw the accumulated_entry and start accumulating a new one
      draw_aggregated(accumulated_entry, false);

      // Must be done before the next `SetEntry` call - we are using the end tick value of the
      // current entry to calculate the next pixel border.
      next_pixel_start_ns = orbit_client_data::GetNextPixelBoundaryTimeNs(
          aggr.GetAccumulatedEntry()->end_tick, resolution_in_pixels, min_tick, max_tick);

      aggr.SetEntry(prev_time, curr_time, curr_normalized_values);
    }

    last_entry_values = curr_normalized_values;
  }

  bool is_accumulated_entry_last = aggr.GetAccumulatedEntry()->end_tick >= max_tick;
  // Draw the leftover entry
  auto accumulated_entry = *aggr.GetAccumulatedEntry();
  draw_aggregated(accumulated_entry, is_accumulated_entry_last);

  // If there was not enough data to reach the end tick, draw an entry until the
  // end.
  if (!is_accumulated_entry_last) {
    DrawSingleSeriesEntry(primitive_assembler, accumulated_entry.end_tick, max_tick,
                          prev_drawn_values, prev_drawn_values, z, true);
  }
}

void LineGraphTrack::DrawSingleSeriesEntry(PrimitiveAssembler& primitive_assembler,
                                           uint64_t start_tick, uint64_t end_tick,
                                           absl::Span<const float> prev_normalized_values,
                                           absl::Span<const float> curr_normalized_values, float z,
                                           bool is_last) {
  constexpr float kDotRadius = 2.f;
  float x0 = timeline_info_->GetWorldFromTick(start_tick);
  float x1 = timeline_info_->GetWorldFromTick(end_tick);
  float content_height = GetGraphContentHeight();
  float base_y = GetGraphContentBottomY();

  for (size_t i = series_.GetDimension(); i-- > 0;) {
    float y0 = base_y - prev_normalized_values[i] * content_height;
    DrawSquareDot(primitive_assembler, Vec2(x0, y0), kDotRadius, z, GetColor(i));
    primitive_assembler.AddLine(Vec2(x0, y0), Vec2(x1, y0), z, GetColor(i));
    if (!is_last) {
      float y1 = base_y - curr_normalized_values[i] * content_height;
      primitive_assembler.AddLine(Vec2(x1, y0), Vec2(x1, y1), z, GetColor(i));
    }
  }
}

}  // namespace orbit_gl
