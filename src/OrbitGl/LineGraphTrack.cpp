// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LineGraphTrack.h"

#include <GteVector.h>

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "Geometry.h"
#include "TextRenderer.h"
#include "TimeGraphLayout.h"

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
void LineGraphTrack<Dimension>::DrawSeries(Batcher& batcher, uint64_t min_tick, uint64_t max_tick,
                                           float z) {
  auto entries = this->series_.GetEntriesAffectedByTimeRange(min_tick, max_tick);
  if (entries.empty()) return;

  double min = this->GetGraphMinValue();
  double inverse_value_range = this->GetInverseOfGraphValueRange();

  auto current_iterator = entries.begin();
  auto last_iterator = std::prev(entries.end());
  uint64_t current_time = current_iterator->first;
  std::array<float, Dimension> current_normalized_values =
      GetNormalizedValues(current_iterator->second, min, inverse_value_range);

  while (current_iterator != last_iterator) {
    auto next_iterator = std::next(current_iterator);
    uint64_t next_time = next_iterator->first;
    std::array<float, Dimension> next_normalized_values =
        GetNormalizedValues(next_iterator->second, min, inverse_value_range);
    bool is_last = next_time >= max_tick;

    DrawSingleSeriesEntry(batcher, current_time, next_time, current_normalized_values,
                          next_normalized_values, z, is_last);

    current_iterator = next_iterator;
    current_time = next_time;
    current_normalized_values = next_normalized_values;
  }

  if (current_time < max_tick) {
    DrawSingleSeriesEntry(batcher, current_time, max_tick, current_normalized_values,
                          current_normalized_values, z, true);
  }
}

static void DrawSquareDot(Batcher& batcher, Vec2 center, float radius, float z,
                          const Color& color) {
  Vec2 position(center[0] - radius, center[1] - radius);
  Vec2 size(2 * radius, 2 * radius);
  batcher.AddBox(Box(position, size, z), color);
}

template <size_t Dimension>
void LineGraphTrack<Dimension>::DrawSingleSeriesEntry(
    Batcher& batcher, uint64_t start_tick, uint64_t end_tick,
    const std::array<float, Dimension>& current_normalized_values,
    const std::array<float, Dimension>& next_normalized_values, float z, bool is_last) {
  constexpr float kDotRadius = 2.f;
  float x0 = this->timeline_info_->GetWorldFromTick(start_tick);
  float x1 = this->timeline_info_->GetWorldFromTick(end_tick);
  float content_height = this->GetGraphContentHeight();
  float base_y = this->GetGraphContentBottomY();

  for (size_t i = Dimension; i-- > 0;) {
    float y0 = base_y - current_normalized_values[i] * content_height;
    DrawSquareDot(batcher, Vec2(x0, y0), kDotRadius, z, this->GetColor(i));
    batcher.AddLine(Vec2(x0, y0), Vec2(x1, y0), z, this->GetColor(i));
    if (!is_last) {
      float y1 = base_y - next_normalized_values[i] * content_height;
      batcher.AddLine(Vec2(x1, y0), Vec2(x1, y1), z, this->GetColor(i));
    }
  }
}

template class LineGraphTrack<1>;
template class LineGraphTrack<2>;
template class LineGraphTrack<3>;
template class LineGraphTrack<4>;

}  // namespace orbit_gl