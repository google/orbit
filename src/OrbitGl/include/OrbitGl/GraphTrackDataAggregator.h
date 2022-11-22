// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_
#define ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

// A utility class to aggregate data points on graph tracks.
// To be consistent with `GetEntriesAffectedByTimeRange`, the time ranges are
// closed, i.e. range [1, 1] is considered 1 tick wide.
template <size_t Dimension>
class GraphTrackDataAggregator {
 public:
  using ValuesT = std::array<float, Dimension>;

  // Starts aggregating a new entry with `values` that start at
  // `start_tick` and end at `end_tick`. The previous aggregated entry is
  // overwritten.
  void SetEntry(uint64_t start_tick, uint64_t end_tick, const ValuesT& values);

  // Merges `values` for the range [`start_tick`, `end_tick`] to the entry.
  // If there is no entry, starts a new one.
  void MergeDataIntoEntry(uint64_t start_tick, uint64_t end_tick, const ValuesT& values);

  // Returns the currently accumulated entry, if any.
  struct AccumulatedEntry {
    uint64_t start_tick = 0;
    uint64_t end_tick = 0;
    ValuesT min_vals{};
    ValuesT max_vals{};
  };
  [[nodiscard]] const AccumulatedEntry* GetAccumulatedEntry() const;

 private:
  std::optional<AccumulatedEntry> accumulated_entry_;
};

#endif  // ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_
