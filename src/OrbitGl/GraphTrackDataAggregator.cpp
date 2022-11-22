// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/GraphTrackDataAggregator.h"

#include <algorithm>

#include "OrbitBase/Logging.h"

template <size_t Dimension>
using ValuesT = typename GraphTrackDataAggregator<Dimension>::ValuesT;

namespace {
template <size_t Dimension>
void MergeValuesWithMax(const ValuesT<Dimension>& src, ValuesT<Dimension>& dest) {
  for (size_t i = 0; i < Dimension; ++i) {
    dest[i] = std::max(dest[i], src[i]);
  }
}

template <size_t Dimension>
void MergeValuesWithMin(const ValuesT<Dimension>& src, ValuesT<Dimension>& dest) {
  for (size_t i = 0; i < Dimension; ++i) {
    dest[i] = std::min(dest[i], src[i]);
  }
}
}  // namespace

template <size_t Dimension>
void GraphTrackDataAggregator<Dimension>::SetEntry(uint64_t start_tick, uint64_t end_tick,
                                                   const ValuesT& values) {
  ORBIT_CHECK(start_tick <= end_tick);

  accumulated_entry_ = AccumulatedEntry{start_tick, end_tick, values, values};
}

template <size_t Dimension>
void GraphTrackDataAggregator<Dimension>::MergeDataIntoEntry(uint64_t start_tick, uint64_t end_tick,
                                                             const ValuesT& values) {
  if (!accumulated_entry_.has_value()) {
    SetEntry(start_tick, end_tick, values);
    return;
  }

  ORBIT_CHECK(start_tick <= end_tick);

  MergeValuesWithMin<Dimension>(values, accumulated_entry_->min_vals);
  MergeValuesWithMax<Dimension>(values, accumulated_entry_->max_vals);

  accumulated_entry_->start_tick = std::min(accumulated_entry_->start_tick, start_tick);
  accumulated_entry_->end_tick = std::max(accumulated_entry_->end_tick, end_tick);
}

template <size_t Dimension>
[[nodiscard]] const typename GraphTrackDataAggregator<Dimension>::AccumulatedEntry*
GraphTrackDataAggregator<Dimension>::GetAccumulatedEntry() const {
  if (accumulated_entry_.has_value()) return &accumulated_entry_.value();
  return nullptr;
}

template class GraphTrackDataAggregator<1>;
template class GraphTrackDataAggregator<2>;
template class GraphTrackDataAggregator<3>;
template class GraphTrackDataAggregator<4>;
