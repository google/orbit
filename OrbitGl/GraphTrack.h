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
  explicit GraphTrack(TimeGraph* time_graph);
  Type GetType() const override { return kGraphTrack; }
  void Draw(GlCanvas* canvas, PickingMode picking_mode) override;
  void OnDrag(int x, int y) override;
  void AddTimer(const Timer& timer) override;
  float GetHeight() const override;

 protected:
  std::map<uint64_t, double> values_;
  double min_ = std::numeric_limits<double>::max();
  double max_ = std::numeric_limits<double>::lowest();
  double value_range_ = 0;
  double inv_value_range_ = 0;
};

#endif
