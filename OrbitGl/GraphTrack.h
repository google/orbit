// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_H
#define ORBIT_GL_GRAPH_TRACK_H

#include <limits>

#include "ScopeTimer.h"
#include "Track.h"

class TimeGraph;

class GraphTrack : public Track {
 public:
  explicit GraphTrack(TimeGraph* time_graph, uint64_t graph_id);
  [[nodiscard]] Type GetType() const override { return kGraphTrack; }
  void Draw(GlCanvas* canvas, PickingMode /*picking_mode*/) override;
  [[nodiscard]] float GetHeight() const override;
  void AddValue(double value, uint64_t time);
  double GetValueAtTime(uint64_t time, double default_value = 0) const;

 protected:
  std::map<uint64_t, double> values_;
  double min_ = std::numeric_limits<double>::max();
  double max_ = std::numeric_limits<double>::lowest();
  double value_range_ = 0;
  double inv_value_range_ = 0;
  uint64_t graph_id_ = 0;
};

#endif
