// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_VARIABLE_TRACK_H_
#define ORBIT_GL_VARIABLE_TRACK_H_

#include <string>
#include <utility>

#include "OrbitGl/LineGraphTrack.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

class VariableTrack final : public LineGraphTrack {
  static constexpr size_t kVariableTrackDimension = 1;
  static constexpr uint8_t kTrackValueDecimalDigits = 6;
  static constexpr const char* kTrackValueUnits = "";

 public:
  explicit VariableTrack(CaptureViewElement* parent,
                         const orbit_gl::TimelineInfoInterface* timeline_info,
                         orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string name,
                         const orbit_client_data::ModuleManager* module_manager,
                         const orbit_client_data::CaptureData* capture_data)
      : LineGraphTrack(parent, timeline_info, viewport, layout,
                       std::vector<std::string>(kVariableTrackDimension), kTrackValueDecimalDigits,
                       kTrackValueUnits, module_manager, capture_data),
        name_{std::move(name)} {
    std::vector<Color> variable_track_color{Color(0, 128, 255, 128)};
    SetSeriesColors(std::move(variable_track_color));
  }

  [[nodiscard]] bool IsCollapsible() const override { return false; }
  [[nodiscard]] std::string GetName() const override { return name_; }
  [[nodiscard]] Track::Type GetType() const override { return Track::Type::kVariableTrack; }
  void AddValue(uint64_t time, double value) { AddValues(time, {value}); }

 private:
  std::string name_;

  std::string GetLegendTooltips(size_t /*legend_index*/) const override { return ""; }
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_VARIABLE_TRACK_H_
