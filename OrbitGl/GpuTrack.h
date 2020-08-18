// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_GPU_TRACK_H_
#define ORBIT_GL_GPU_TRACK_H_

#include <memory>

#include "StringManager.h"
#include "TextBox.h"
#include "Threading.h"
#include "TimerTrack.h"
#include "capture_data.pb.h"

class TextRenderer;

namespace OrbitGl {

// Maps the Linux kernel timeline names (like "gfx", "sdma0") to a more
// descriptive human readable form that is used for the track label.
std::string MapGpuTimelineToTrackLabel(std::string_view timeline);

}  // namespace OrbitGl

class GpuTrack : public TimerTrack {
 public:
  GpuTrack(TimeGraph* time_graph, std::shared_ptr<StringManager> string_manager,
           uint64_t timeline_hash);
  ~GpuTrack() override = default;
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] Type GetType() const override { return kGpuTrack; }
  [[nodiscard]] float GetHeight() const override;

  [[nodiscard]] const TextBox* GetLeft(const TextBox* text_box) const override;
  [[nodiscard]] const TextBox* GetRight(const TextBox* text_box) const override;

  [[nodiscard]] float GetYFromDepth(uint32_t depth) const override;

 protected:
  [[nodiscard]] bool IsTimerActive(const orbit_client_protos::TimerInfo& timer) const override;
  [[nodiscard]] Color GetTimerColor(const orbit_client_protos::TimerInfo& timer,
                                    bool is_selected) const override;
  [[nodiscard]] bool TimerFilter(const orbit_client_protos::TimerInfo& timer) const override;
  void SetTimesliceText(const orbit_client_protos::TimerInfo& timer, double elapsed_us, float min_x,
                        TextBox* text_box) override;
  [[nodiscard]] std::string GetBoxTooltip(PickingId id) const override;

 private:
  uint64_t timeline_hash_;
  std::shared_ptr<StringManager> string_manager_;
  [[nodiscard]] std::string GetSwQueueTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetHwQueueTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::string GetHwExecutionTooltip(
      const orbit_client_protos::TimerInfo& timer_info) const;
};

#endif  // ORBIT_GL_GPU_TRACK_H_
