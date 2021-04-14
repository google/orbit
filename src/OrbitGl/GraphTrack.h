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
  explicit GraphTrack(CaptureViewElement* parent, TimeGraph* time_graph, TimeGraphLayout* layout,
                      std::string name, const CaptureData* capture_data);
  [[nodiscard]] Type GetType() const override { return Type::kGraphTrack; }
  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;
  [[nodiscard]] float GetHeight() const override;
  void AddValue(double value, uint64_t time);
  [[nodiscard]] std::optional<std::pair<uint64_t, double>> GetPreviousValueAndTime(
      uint64_t time) const;
  [[nodiscard]] bool IsEmpty() const override { return values_.empty(); }

  void SetLabelUnitWhenEmpty(const std::string& label_unit);
  void SetValueDecimalDigitsWhenEmpty(uint8_t value_decimal_digits);

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  std::vector<std::shared_ptr<TimerChain>> GetAllChains() const override;
  std::vector<std::shared_ptr<TimerChain>> GetAllSerializableChains() const override {
    return GetAllChains();
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
  std::optional<uint8_t> value_decimal_digits_ = std::nullopt;
  std::string label_unit_;
};

#endif  // ORBIT_GL_GRAPH_TRACK_H_
