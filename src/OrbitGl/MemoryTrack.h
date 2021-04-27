// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MEMORY_TRACK_H_
#define ORBIT_GL_MEMORY_TRACK_H_

#include <string>
#include <utility>

#include "GraphTrack.h"
#include "Track.h"

namespace orbit_gl {

class MemoryTrack final : public GraphTrack {
 public:
  explicit MemoryTrack(CaptureViewElement* parent, TimeGraph* time_graph, TimeGraphLayout* layout,
                       std::string name, const orbit_client_model::CaptureData* capture_data)
      : GraphTrack(parent, time_graph, layout, std::move(name), capture_data) {}
  ~MemoryTrack() override = default;
  [[nodiscard]] Type GetType() const override { return Type::kMemoryTrack; }

  void Draw(GlCanvas* canvas, PickingMode picking_mode, float z_offset = 0) override;

  void SetWarningThresholdWhenEmpty(const std::string& pretty_label, double raw_value);
  void SetValueUpperBoundWhenEmpty(const std::string& pretty_label, double raw_value);
  void SetValueLowerBoundWhenEmpty(const std::string& pretty_label, double raw_value);

 private:
  void UpdateMinAndMax(double value);

  std::optional<std::pair<std::string, double>> warning_threshold_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_upper_bound_ = std::nullopt;
  std::optional<std::pair<std::string, double>> value_lower_bound_ = std::nullopt;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MEMORY_TRACK_H_