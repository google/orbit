// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_
#define ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_

#include <absl/types/span.h>

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace orbit_gl {

// A utility class to aggregate data points on graph tracks.
// To be consistent with `GetEntriesAffectedByTimeRange`, the time ranges are
// closed, i.e. range [1, 1] is considered 1 tick wide.
class GraphTrackDataAggregator {
 public:
  // Starts aggregating a new entry with `values` that start at `start_tick` and end at `end_tick`.
  // The previous aggregated entry is overwritten.
  void SetEntry(uint64_t start_tick, uint64_t end_tick, absl::Span<const float> values);

  // Merges `values` for the range [`start_tick`, `end_tick`] to the entry with checking whether
  // `values` has the same size as the entry. If there is no entry, starts a new one.
  void MergeDataIntoEntry(uint64_t start_tick, uint64_t end_tick, absl::Span<const float> values);

  // Returns the currently accumulated entry, if any.
  struct AccumulatedEntry {
    uint64_t start_tick = 0;
    uint64_t end_tick = 0;
    std::vector<float> min_vals{};
    std::vector<float> max_vals{};
  };
  [[nodiscard]] const AccumulatedEntry* GetAccumulatedEntry() const;

 private:
  std::optional<AccumulatedEntry> accumulated_entry_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_
