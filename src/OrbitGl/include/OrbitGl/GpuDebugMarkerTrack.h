// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_DEBUG_MARKER_TRACK_H_
#define ORBIT_GL_GPU_DEBUG_MARKER_TRACK_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TimerData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/CoreMath.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/TimerTrack.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"
#include "StringManager/StringManager.h"

class OrbitApp;

// This is a thin implementation of a `TimerTrack` to display Vulkan debug markers, used in the
// `GpuTrack`.
class GpuDebugMarkerTrack final : public TimerTrack {
 public:
  explicit GpuDebugMarkerTrack(CaptureViewElement* parent,
                               const orbit_gl::TimelineInfoInterface* timeline_info,
                               orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                               uint64_t timeline_hash, OrbitApp* app,
                               const orbit_client_data::ModuleManager* module_manager,
                               const orbit_client_data::CaptureData* capture_data,
                               orbit_client_data::TimerData* timer_data);

  ~GpuDebugMarkerTrack() override = default;

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetLabel() const override { return "Debug Markers"; }
  // The type is currently only used by the TrackManager. We are moving towards removing it
  // completely. For subtracks there is no meaningful type and it should also not be exposed,
  // though we use the unknown type.
  [[nodiscard]] Type GetType() const override { return Type::kUnknown; }
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] bool IsCollapsible() const override { return GetDepth() > 1; }

  [[nodiscard]] float GetYFromDepth(uint32_t depth) const override;
  [[nodiscard]] bool TimerFilter(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer, bool is_selected,
                                    bool is_highlighted,
                                    const internal::DrawData& draw_data) const override;
  [[nodiscard]] std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& timer) const override;

  [[nodiscard]] std::string GetBoxTooltip(const orbit_gl::PrimitiveAssembler& primitive_assembler,
                                          PickingId id) const override;

 private:
  orbit_string_manager::StringManager* string_manager_;
  uint64_t timeline_hash_;
};

#endif  // ORBIT_GL_GPU_DEBUG_MARKER_TRACK_H_
