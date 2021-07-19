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
#include "ClientData/TextBox.h"
#include "CoreMath.h"
#include "GpuDebugMarkerTrack.h"
#include "GpuSubmissionTrack.h"
#include "PickingManager.h"
#include "StringManager.h"
#include "TimerTrack.h"
#include "Track.h"
#include "Viewport.h"
#include "capture_data.pb.h"

class OrbitApp;
class TextRenderer;

namespace orbit_gl {

// Maps the Linux kernel timeline names (like "gfx", "sdma0") to a more
// descriptive human readable form that is used for the track label.
std::string MapGpuTimelineToTrackLabel(std::string_view timeline);

}  // namespace orbit_gl

// A track to display Gpu related information (of a certain command queue). It contains two
// subtracks to display "submission" related information, as well as "debug markers" (if present).
class GpuTrack : public Track {
 public:
  explicit GpuTrack(CaptureViewElement* parent, TimeGraph* time_graph, orbit_gl::Viewport* viewport,
                    TimeGraphLayout* layout, uint64_t timeline_hash, OrbitApp* app,
                    const orbit_client_model::CaptureData* capture_data,
                    uint32_t indentation_level = 0);
  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& timer_info) const;

  [[nodiscard]] const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& timer_info) const;

  [[nodiscard]] Type GetType() const override { return Type::kGpuTrack; }
  [[nodiscard]] std::string GetTooltip() const override;
  [[nodiscard]] float GetHeight() const override;

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;
  [[nodiscard]] std::vector<CaptureViewElement*> GetVisibleChildren() override;

  [[nodiscard]] bool IsEmpty() const override {
    return submission_track_->IsEmpty() && marker_track_->IsEmpty();
  }
  [[nodiscard]] bool IsCollapsible() const override { return true; }

  [[nodiscard]] uint32_t GetNumTimers() const override {
    return submission_track_->GetNumTimers() + marker_track_->GetNumTimers();
  }

  [[nodiscard]] uint64_t GetMinTime() const override {
    return std::min(submission_track_->GetMinTime(), marker_track_->GetMinTime());
  }

  [[nodiscard]] uint64_t GetMaxTime() const override {
    return std::max(submission_track_->GetMaxTime(), marker_track_->GetMaxTime());
  }

 private:
  void UpdatePositionOfSubtracks();
  const std::shared_ptr<GpuSubmissionTrack> submission_track_;
  const std::shared_ptr<GpuDebugMarkerTrack> marker_track_;

  uint64_t timeline_hash_;
};

#endif  // ORBIT_GL_GPU_TRACK_H_
