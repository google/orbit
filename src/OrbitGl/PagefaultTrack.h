// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PAGEFAULT_TRACK_H_
#define ORBIT_GL_PAGEFAULT_TRACK_H_

#include "MajorPagefaultTrack.h"
#include "MinorPagefaultTrack.h"
#include "Timer.h"
#include "Track.h"
#include "Viewport.h"
#include "capture_data.pb.h"

namespace orbit_gl {

// This track displays pagefault related information for the system, cgroup and process memory
// usage. It contains two subtracks to display major pagefault related information, as well as minor
// pagefault related information.
class PagefaultTrack : public Track {
 public:
  explicit PagefaultTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                          orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                          const std::string& cgroup_name, uint64_t memory_sampling_period_ms,
                          const orbit_client_model::CaptureData* capture_data,
                          uint32_t indentation_level = 0);

  [[nodiscard]] Type GetType() const override { return Type::kPagefaultTrack; }
  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] std::vector<CaptureViewElement*> GetVisibleChildren() override;
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] bool IsEmpty() const override {
    return major_pagefault_track_->IsEmpty() && minor_pagefault_track_->IsEmpty();
  }
  [[nodiscard]] bool IsCollapsible() const override { return true; }

  void Draw(Batcher& batcher, TextRenderer& text_renderer, uint64_t current_mouse_time_ns,
            PickingMode picking_mode, float z_offset = 0) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;
  [[nodiscard]] std::vector<std::shared_ptr<orbit_client_data::TimerChain>> GetAllChains()
      const override;
  [[nodiscard]] std::vector<std::shared_ptr<orbit_client_data::TimerChain>>
  GetAllSerializableChains() const override {
    return GetAllChains();
  }

  void AddValuesAndUpdateAnnotationsForMajorPagefaultSubtrack(
      uint64_t timestamp_ns, const std::array<double, kBasicPagefaultTrackDimension>& values) {
    major_pagefault_track_->AddValuesAndUpdateAnnotations(timestamp_ns, values);
  }
  void AddValuesAndUpdateAnnotationsForMinorPagefaultSubtrack(
      uint64_t timestamp_ns, const std::array<double, kBasicPagefaultTrackDimension>& values) {
    minor_pagefault_track_->AddValuesAndUpdateAnnotations(timestamp_ns, values);
  }
  void SetMemorySamplingPeriodMs(uint64_t memory_sampling_period_ms);

 private:
  void UpdatePositionOfSubtracks();

  std::shared_ptr<MajorPagefaultTrack> major_pagefault_track_;
  std::shared_ptr<MinorPagefaultTrack> minor_pagefault_track_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PAGEFAULT_TRACK_H_
