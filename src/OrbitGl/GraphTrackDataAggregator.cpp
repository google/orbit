// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GraphTrackDataAggregator.h"

#include <algorithm>
#include <cstddef>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

namespace {
void MergeValuesWithMax(absl::Span<const float> src, absl::Span<float> dest) {
  ORBIT_CHECK(src.size() == dest.size());

  for (size_t i = 0; i < dest.size(); ++i) dest[i] = std::max(dest[i], src[i]);
}

void MergeValuesWithMin(absl::Span<const float> src, absl::Span<float> dest) {
  ORBIT_CHECK(src.size() == dest.size());

  for (size_t i = 0; i < dest.size(); ++i) dest[i] = std::min(dest[i], src[i]);
}
}  // namespace

void GraphTrackDataAggregator::SetEntry(uint64_t start_tick, uint64_t end_tick,
                                        absl::Span<const float> values) {
  ORBIT_CHECK(start_tick <= end_tick);

  accumulated_entry_ = AccumulatedEntry{
      start_tick, end_tick, {values.begin(), values.end()}, {values.begin(), values.end()}};
}

void GraphTrackDataAggregator::MergeDataIntoEntry(uint64_t start_tick, uint64_t end_tick,
                                                  absl::Span<const float> values) {
  if (!accumulated_entry_.has_value()) {
    SetEntry(start_tick, end_tick, values);
    return;
  }

  ORBIT_CHECK(start_tick <= end_tick);

  MergeValuesWithMin(values, absl::MakeSpan(accumulated_entry_->min_vals));
  MergeValuesWithMax(values, absl::MakeSpan(accumulated_entry_->max_vals));

  accumulated_entry_->start_tick = std::min(accumulated_entry_->start_tick, start_tick);
  accumulated_entry_->end_tick = std::max(accumulated_entry_->end_tick, end_tick);
}

[[nodiscard]] const GraphTrackDataAggregator::AccumulatedEntry*
GraphTrackDataAggregator::GetAccumulatedEntry() const {
  if (accumulated_entry_.has_value()) return &accumulated_entry_.value();

  return nullptr;
}

}  // namespace orbit_gl