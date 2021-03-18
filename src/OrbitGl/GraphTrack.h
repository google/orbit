// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GRAPH_TRACK_H_
#define ORBIT_GL_GRAPH_TRACK_H_

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
  explicit GraphTrack(TimeGraph* time_graph, TimeGraphLayout* layout, std::string name,
                      const CaptureData* capture_data);
  [[nodiscard]] Type GetType() const override { return kGraphTrack; }
  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;
  [[nodiscard]] float GetHeight() const override;
  void AddValue(double value, uint64_t time);
  [[nodiscard]] std::optional<std::pair<uint64_t, double>> GetPreviousValueAndTime(
      uint64_t time) const;
  [[nodiscard]] bool IsEmpty() const override { return values_.empty(); }

  void SetWarningThresholdWhenEmpty(
      const std::optional<std::pair<std::string, double>>& warning_threshold) {
    if (warning_threshold_.has_value() || !warning_threshold.has_value()) return;
    warning_threshold_ = warning_threshold;
    max_ = std::max(max_, warning_threshold.value().second);
    min_ = std::min(min_, warning_threshold.value().second);
  }
  void SetValueUpperBoundWhenEmpty(
      const std::optional<std::pair<std::string, double>>& value_upper_bound) {
    if (value_upper_bound_.has_value() || !value_upper_bound.has_value()) return;
    value_upper_bound_ = value_upper_bound;
  }

 protected:
  void DrawSquareDot(Batcher* batcher, Vec2 center, float radius, float z, const Color& color);
  void DrawLabel(GlCanvas* canvas, Vec2 target_pos, const std::string& text,
                 const Color& text_color, const Color& font_color, float z);
  std::map<uint64_t, double> values_;
  double min_ = std::numeric_limits<double>::max();
  double max_ = std::numeric_limits<double>::lowest();
  double value_range_ = 0;
  double inv_value_range_ = 0;
  std::optional<std::pair<std::string, double>> warning_threshold_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_upper_bound_ = std::nullopt;
};

#endif  // ORBIT_GL_GRAPH_TRACK_H_
