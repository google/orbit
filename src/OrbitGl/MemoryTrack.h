// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MEMORY_TRACK_H_
#define ORBIT_GL_MEMORY_TRACK_H_

#include <string>
#include <utility>

#include "Track.h"
#include "VariableTrack.h"
#include "Viewport.h"

namespace orbit_gl {

class MemoryTrack final : public VariableTrack {
 public:
  explicit MemoryTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                       const orbit_client_model::CaptureData* capture_data)
      : VariableTrack(parent, time_graph, viewport, layout, name, capture_data) {}
  ~MemoryTrack() override = default;
  [[nodiscard]] Type GetType() const override { return Type::kMemoryTrack; }

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset = 0) override;

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