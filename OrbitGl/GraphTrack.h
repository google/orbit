// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_H
#define ORBIT_GL_GRAPH_TRACK_H

#include <stdint.h>

#include <limits>
#include <map>
#include <optional>
#include <string>
#include <utility>

#include "Batcher.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "Timer.h"
#include "Track.h"

class TimeGraph;

class GraphTrack : public Track {
 public:
  explicit GraphTrack(TimeGraph* time_graph, std::string name);
  [[nodiscard]] Type GetType() const override { return kGraphTrack; }
  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode,
                        float z_offset = 0) override;
  [[nodiscard]] float GetHeight() const override;
  void AddValue(double value, uint64_t time);
  [[nodiscard]] std::optional<std::pair<uint64_t, double> > GetPreviousValueAndTime(
      uint64_t time) const;
  [[nodiscard]] bool IsEmpty() const override { return values_.empty(); }

 protected:
  void DrawSquareDot(Batcher* batcher, Vec2 center, float radius, float z, Color color);
  void DrawLabel(GlCanvas* canvas, Vec2 target_pos, std::string text, Color text_color,
                 Color font_color, float z);
  std::map<uint64_t, double> values_;
  double min_ = std::numeric_limits<double>::max();
  double max_ = std::numeric_limits<double>::lowest();
  double value_range_ = 0;
  double inv_value_range_ = 0;
  std::string name_;
};

#endif
