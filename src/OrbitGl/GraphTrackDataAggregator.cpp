// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "GraphTrackDataAggregator.h"

template <size_t Dimension>
void GraphTrackDataAggregator<Dimension>::StartNewEntry(uint64_t start_tick, uint64_t end_tick,
                                                        const ValuesT& values) {
  ORBIT_CHECK(start_tick <= end_tick);

  entry_ = Entry{start_tick, end_tick, values};
}

template <size_t Dimension>
void GraphTrackDataAggregator<Dimension>::AppendWithAveraging(uint64_t start_tick,
                                                              uint64_t end_tick,
                                                              const ValuesT& values) {
  uint64_t new_val_duration = end_tick - start_tick + 1;
  uint64_t curr_val_duration = entry_.end_tick - entry_.start_tick + 1;
  float append_weight = new_val_duration / curr_val_duration;
  float total_weight = 1.f + append_weight;
  for (size_t i = 0; i < Dimension; ++i) {
    entry_.values[i] += values[i] * append_weight;
    entry_.values[i] /= total_weight;
  }
}

template <size_t Dimension>
void GraphTrackDataAggregator<Dimension>::AppendWithMaxValue(const ValuesT& values) {
  for (size_t i = 0; i < Dimension; ++i) {
    entry_.values[i] = std::max(values[i], entry_.values[i]);
  }
}

// Append `values` for the range [`start_tick`, `end_tick`] to the entry
// we're accumulating. The values are aggregated depending on the mode set
// in the constructor.
template <size_t Dimension>
void GraphTrackDataAggregator<Dimension>::AppendData(uint64_t start_tick, uint64_t end_tick,
                                                     const ValuesT& values) {
  ORBIT_CHECK(entry_.end_tick <= start_tick);
  ORBIT_CHECK(start_tick <= end_tick);

  switch (mode_) {
    case GraphTrackAggregationMode::kMax:
      AppendWithMaxValue(values);
      break;
    case GraphTrackAggregationMode::kAvg:
      AppendWithAveraging(start_tick, end_tick, values);
      break;
  }
  entry_.end_tick = end_tick;
}

// Return the currently accumulated entry.
template <size_t Dimension>
[[nodiscard]] const typename GraphTrackDataAggregator<Dimension>::Entry&
GraphTrackDataAggregator<Dimension>::GetEntry() const {
  return entry_;
}

template class GraphTrackDataAggregator<1>;
template class GraphTrackDataAggregator<2>;
template class GraphTrackDataAggregator<3>;
template class GraphTrackDataAggregator<4>;
