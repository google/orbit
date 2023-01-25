// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_LINE_GRAPH_TRACK_H_
#define ORBIT_GL_LINE_GRAPH_TRACK_H_

#include <absl/types/span.h>
#include <stddef.h>
#include <stdint.h>

#include <string>
#include <tuple>
#include <utility>

#include "OrbitGl/GraphTrack.h"
#include "OrbitGl/PrimitiveAssembler.h"

namespace orbit_gl {

// This is an implementation of the `GraphTrack` to visualize the `MultivariateTimeSeries` data in a
// multi-line stairstep graph.
class LineGraphTrack : public GraphTrack {
 public:
  using GraphTrack::GraphTrack;
  ~LineGraphTrack() override = default;

 protected:
  [[nodiscard]] float GetLabelYFromValues(absl::Span<const double> values) const override;
  void DrawSeries(PrimitiveAssembler& primitive_assembler, uint64_t min_tick, uint64_t max_tick,
                  float z) override;
  virtual void DrawSingleSeriesEntry(PrimitiveAssembler& primitive_assembler, uint64_t start_tick,
                                     uint64_t end_tick,
                                     absl::Span<const float> prev_normalized_values,
                                     absl::Span<const float> curr_normalized_values, float z,
                                     bool is_last);

  // Determines how values should be aggregated for drawing.
  enum class AggregationMode {
    // Draw only the max value of each element. Less visual noise but may lose min values.
    kMax = 0,
    // Draw both min and max values. May be noisy, but preserves both up and down spikes.
    kMinMax,
  };
  AggregationMode aggregation_mode_ = AggregationMode::kMinMax;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_LINE_GRAPH_TRACK_H_
