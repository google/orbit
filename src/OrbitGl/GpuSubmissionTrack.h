// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_SUBMISSION_TRACK_H_
#define ORBIT_GL_GPU_SUBMISSION_TRACK_H_

#include <stdint.h>

#include <memory>
#include <string>
#include <string_view>

#include "CallstackThreadBar.h"
#include "CoreMath.h"
#include "GpuDebugMarkerTrack.h"
#include "PickingManager.h"
#include "StringManager.h"
#include "TextBox.h"
#include "TimerTrack.h"
#include "Track.h"
#include "capture_data.pb.h"

class OrbitApp;
class TextRenderer;

// This track displays the "vkQueueSubmit"s including its hardware execution times and -- if
// available -- command buffer timings on a certain command queue. In order to visually separate
// different submissions that would overlap, they are displayed on a different depth with a thin gap
// between them.
// This track is meant to be used as a subtrack of `GpuTrack`.
class GpuSubmissionTrack : public TimerTrack {
 public:
  explicit GpuSubmissionTrack(Track* parent, TimeGraph* time_graph, orbit_gl::Viewport* viewport,
                              TimeGraphLayout* layout, uint64_t timeline_hash, OrbitApp* app,
                              const orbit_client_model::CaptureData* capture_data,
                              uint32_t indentation_level);
  ~GpuSubmissionTrack() override = default;

  [[nodiscard]] Track* GetParent() const override { return parent_; }

  // The type is currently only used by the TrackManger. We are moving towards removing it
  // completely. For subtracks there is no meaningful type and it should also not be exposed,
  // though we use the unknown type.
  [[nodiscard]] Type GetType() const override { return Type::kUnknown; }
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] float GetHeight() const override;

  [[nodiscard]] const TextBox* GetLeft(const TextBox* text_box) const override;
  [[nodiscard]] const TextBox* GetRight(const TextBox* text_box) const override;

  [[nodiscard]] float GetYFromTimer(
      const orbit_client_protos::TimerInfo& timer_info) const override;

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] bool IsCollapsible() const override {
    return depth_ > 1 || has_vulkan_layer_command_buffer_timers_;
  }

 protected:
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer, bool is_selected,
                                    bool is_highlighted) const override;
  [[nodiscard]] bool TimerFilter(const orbit_client_protos::TimerInfo& timer) const override;
  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, float min_x, float z_offset,
                        TextBox* text_box) override;
  [[nodiscard]] std::string GetBoxTooltip(const Batcher& batcher, PickingId id) const override;

 private:
  uint64_t timeline_hash_;
  StringManager* string_manager_;
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
