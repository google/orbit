// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_SUBMISSION_TRACK_H_
#define ORBIT_GL_GPU_SUBMISSION_TRACK_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TimerData.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
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

// This track displays the "vkQueueSubmit"s including its hardware execution times and -- if
// available -- command buffer timings on a certain command queue. In order to visually separate
// different submissions that would overlap, they are displayed on a different depth with a thin gap
// between them.
// This track is meant to be used as a subtrack of `GpuTrack`.
class GpuSubmissionTrack : public TimerTrack {
 public:
  explicit GpuSubmissionTrack(Track* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
                              orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                              uint64_t timeline_hash, OrbitApp* app,
                              const orbit_client_data::ModuleManager* module_manager,
                              const orbit_client_data::CaptureData* capture_data,
                              orbit_client_data::TimerData* timer_data);
  ~GpuSubmissionTrack() override = default;

  [[nodiscard]] Track* GetParent() const override { return parent_; }

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetLabel() const override { return "Submissions"; }
  // The type is currently only used by the TrackManger. We are moving towards removing it
  // completely. For subtracks there is no meaningful type and it should also not be exposed,
  // though we use the unknown type.
  [[nodiscard]] Type GetType() const override { return Type::kUnknown; }
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] float GetHeight() const override;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer_info) const override;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] bool IsCollapsible() const override {
    return GetDepth() > 1 || has_vulkan_layer_command_buffer_timers_;
  }
  [[nodiscard]] bool IsCollapsed() const override {
    return Track::IsCollapsed() || GetParent()->IsCollapsed();
  }

 protected:
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer, bool is_selected,
                                    bool is_highlighted,
                                    const internal::DrawData& draw_data) const override;
  [[nodiscard]] bool TimerFilter(const orbit_client_protos::TimerInfo& timer) const override;

  [[nodiscard]] std::string GetTimesliceText(
      const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] std::string GetBoxTooltip(const orbit_gl::PrimitiveAssembler& primitive_assembler,
                                          PickingId id) const override;

 private:
  uint64_t timeline_hash_;
  orbit_string_manager::StringManager* string_manager_;
  Track* parent_;

  bool has_vulkan_layer_command_buffer_timers_ = false;
  [[nodiscard]] std::string GetSwQueueTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetHwQueueTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetHwExecutionTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetCommandBufferTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
};

#endif  // ORBIT_GL_GPU_SUBMISSION_TRACK_H_
