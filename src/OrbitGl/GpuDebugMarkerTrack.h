// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_DEBUG_MARKER_TRACK_H_
#define ORBIT_GL_GPU_DEBUG_MARKER_TRACK_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>

#include "CallstackThreadBar.h"
#include "CoreMath.h"
#include "PickingManager.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TimerTrack.h"
#include "Track.h"
#include "capture_data.pb.h"

class OrbitApp;
class TextRenderer;

// This is a thin implementation of a `TimerTrack` to display Vulkan debug markers, used in the
// `GpuTrack`.
class GpuDebugMarkerTrack : public TimerTrack {
 public:
  explicit GpuDebugMarkerTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                               const orbit_client_model::CaptureData* capture_data,
                               uint32_t indentation_level);

  ~GpuDebugMarkerTrack() override = default;

  // The type is currently only used by the TrackManger. We are moving towards removing it
  // completely. For subtracks there is no meaningful type and it should also not be exposed,
  // though we use the unknown type.
  [[nodiscard]] Type GetType() const override { return Type::kUnknown; }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] float GetHeight() const override;

  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] bool TimerFilter(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer, bool is_selected,
                                    bool is_highlighted) const override;
  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, float min_x, float z_offset,
                        TextBox* text_box) override;

  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;

 private:
  StringManager* string_manager_;
};

#endif  // ORBIT_GL_GPU_DEBUG_MARKER_TRACK_H_
