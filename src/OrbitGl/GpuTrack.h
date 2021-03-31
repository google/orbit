// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_TRACK_H_
#define ORBIT_GL_GPU_TRACK_H_

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

namespace orbit_gl {

// Maps the Linux kernel timeline names (like "gfx", "sdma0") to a more
// descriptive human readable form that is used for the track label.
std::string MapGpuTimelineToTrackLabel(std::string_view timeline);

}  // namespace orbit_gl

class GpuTrack : public TimerTrack {
 public:
  explicit GpuTrack(TimeGraph* time_graph, TimeGraphLayout* layout, uint64_t timeline_hash,
                    OrbitApp* app, const CaptureData* capture_data);
  ~GpuTrack() override = default;
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] Type GetType() const override { return kGpuTrack; }
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
  bool has_vulkan_layer_command_buffer_timers_ = false;
  [[nodiscard]] std::string GetSwQueueTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetHwQueueTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetHwExecutionTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetCommandBufferTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetDebugMarkerTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
};

#endif  // ORBIT_GL_GPU_TRACK_H_
