// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PAGE_FAULTS_TRACK_H_
#define ORBIT_GL_PAGE_FAULTS_TRACK_H_

#include "MajorPageFaultsTrack.h"
#include "MinorPageFaultsTrack.h"
#include "Timer.h"
#include "Track.h"
#include "Viewport.h"
#include "capture_data.pb.h"

namespace orbit_gl {

// This track displays page faults related information for the system, cgroup and process memory
// usage. It contains two subtracks to display major page faults related information, as well as
// minor page faults related information.
class PageFaultsTrack : public Track {
 public:
  explicit PageFaultsTrack(CaptureViewElement* parent, TimeGraph* time_graph,
                           orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                           const std::string& cgroup_name, uint64_t memory_sampling_period_ms,
                           const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override { return "Page Faults"; }
  [[nodiscard]] Type GetType() const override { return Type::kPageFaultsTrack; }
  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] std::vector<CaptureViewElement*> GetVisibleChildren() override;
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] bool IsEmpty() const override {
    return major_page_faults_track_->IsEmpty() && minor_page_faults_track_->IsEmpty();
  }
  [[nodiscard]] bool IsCollapsible() const override { return true; }

  void Draw(Batcher& batcher, TextRenderer& text_renderer,
            const DrawContext& draw_context) override;
  void UpdatePrimitives(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                        PickingMode picking_mode, float z_offset = 0) override;

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

  void AddValuesAndUpdateAnnotationsForMajorPageFaultsSubtrack(
      uint64_t timestamp_ns, const std::array<double, kBasicPageFaultsTrackDimension>& values) {
    major_page_faults_track_->AddValuesAndUpdateAnnotations(timestamp_ns, values);
  }

  void AddValuesAndUpdateAnnotationsForMinorPageFaultsSubtrack(
      uint64_t timestamp_ns, const std::array<double, kBasicPageFaultsTrackDimension>& values) {
    minor_page_faults_track_->AddValuesAndUpdateAnnotations(timestamp_ns, values);
  }

  [[nodiscard]] uint64_t GetMinTime() const override;
  [[nodiscard]] uint64_t GetMaxTime() const override;

 private:
  void UpdatePositionOfSubtracks();

  std::shared_ptr<MajorPageFaultsTrack> major_page_faults_track_;
  std::shared_ptr<MinorPageFaultsTrack> minor_page_faults_track_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PAGE_FAULTS_TRACK_H_
