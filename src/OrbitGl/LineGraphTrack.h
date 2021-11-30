// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_LINE_GRAPH_TRACK_H_
#define ORBIT_GL_LINE_GRAPH_TRACK_H_

#include <string>
#include <utility>

#include "GraphTrack.h"

namespace orbit_gl {

// This is an implementation of the `GraphTrack` to visualize the `MultivariateTimeSeries` data in a
// multi-line stairstep graph.
template <size_t Dimension>
class LineGraphTrack : public GraphTrack<Dimension> {
 public:
  using GraphTrack<Dimension>::GraphTrack;
  ~LineGraphTrack() override = default;

 protected:
  [[nodiscard]] float GetLabelYFromValues(
      const std::array<double, Dimension>& values) const override;
  void DrawSeries(Batcher& batcher, uint64_t min_tick, uint64_t max_tick, float z) override;
  virtual void DrawSingleSeriesEntry(Batcher& batcher, uint64_t start_tick, uint64_t end_tick,
                                     const std::array<float, Dimension>& current_normalized_values,
                                     const std::array<float, Dimension>& next_normalized_values,
                                     float z, bool is_last);
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_LINE_GRAPH_TRACK_H_