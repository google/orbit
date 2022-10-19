// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_
#define ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_

#include <array>
#include <cstdint>

#include "OrbitBase/Logging.h"

// Determines how two value arrays should be aggregated.
enum class GraphTrackAggregationMode {
  kMax = 0, // Use the max value of each element.
  kAvg, // Use weighted average. The weights are determined by time ranges.
};

// A utility class to aggregate data points on graph tracks using a selected
// mode.
// To be consistent with `GetEntriesAffectedByTimeRange`, the time ranges are
// closed, i.e. range [1, 1] is considered 1 tick wide.
template<size_t Dimension>
class GraphTrackDataAggregator {
  public:
    explicit GraphTrackDataAggregator(GraphTrackAggregationMode mode) : mode_{mode} {}

    using ValuesT = std::array<float, Dimension>;
    struct Entry {
      uint64_t start_tick = 0;
      uint64_t end_tick = 0;
      ValuesT values{};
    };

    // Start aggregating a new entry with `values` that start at
    // `start_tick` and end at `end_tick`. The previous aggregated entry is
    // overwritten.
    void StartNewEntry(uint64_t start_tick, uint64_t end_tick, const ValuesT& values);

    // Append `values` for the range [`start_tick`, `end_tick`] to the entry
    // we're accumulating. The values are aggregated depending on the mode set
    // in the constructor.
    void AppendData(uint64_t start_tick, uint64_t end_tick, const ValuesT& values);

    // Return the currently accumulated entry.
    [[ nodiscard ]] const Entry& GetEntry() const;

  private:
    void AppendWithAveraging(uint64_t start_tick, uint64_t end_tick, const ValuesT& values);
    void AppendWithMaxValue(const ValuesT& values);

    const GraphTrackAggregationMode mode_;
    Entry entry_;
};

#endif  // ORBIT_GL_GRAPH_TRACK_DATA_AGGREGATOR_H_
