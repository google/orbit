// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_PAGE_FAULTS_TRACK_H_
#define ORBIT_GL_PAGE_FAULTS_TRACK_H_

#include <stdint.h>

#include <array>
#include <memory>
#include <string>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PageFaultsInfo.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/BasicPageFaultsTrack.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/MajorPageFaultsTrack.h"
#include "OrbitGl/MinorPageFaultsTrack.h"
#include "OrbitGl/PickingManager.h"
#include "OrbitGl/PrimitiveAssembler.h"
#include "OrbitGl/TextRenderer.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

// This track displays page faults related information for the system, cgroup and process memory
// usage. It contains two subtracks to display major page faults related information, as well as
// minor page faults related information.
class PageFaultsTrack : public Track {
 public:
  explicit PageFaultsTrack(CaptureViewElement* parent,
                           const orbit_gl::TimelineInfoInterface* timeline_info,
                           orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                           std::string cgroup_name, uint64_t memory_sampling_period_ms,
                           const orbit_client_data::ModuleManager* module_manager,
                           const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override { return "Page Faults"; }
  [[nodiscard]] std::string GetLabel() const override;
  [[nodiscard]] Type GetType() const override { return Type::kPageFaultsTrack; }
  [[nodiscard]] float GetHeight() const override;
  [[nodiscard]] std::string GetTooltip() const override;

  [[nodiscard]] bool IsEmpty() const override {
    return major_page_faults_track_->IsEmpty() && minor_page_faults_track_->IsEmpty();
  }
  [[nodiscard]] bool IsCollapsible() const override { return true; }
  [[nodiscard]] std::vector<CaptureViewElement*> GetAllChildren() const override;

  void OnPageFaultsInfo(const orbit_client_data::PageFaultsInfo& page_faults_info);

  void AddValuesAndUpdateAnnotationsForMajorPageFaultsSubtrack(
      uint64_t timestamp_ns, const std::array<double, kBasicPageFaultsTrackDimension>& values) {
    major_page_faults_track_->AddValuesAndUpdateAnnotations(timestamp_ns, values);
  }

  void AddValuesAndUpdateAnnotationsForMinorPageFaultsSubtrack(
      uint64_t timestamp_ns, const std::array<double, kBasicPageFaultsTrackDimension>& values) {
    minor_page_faults_track_->AddValuesAndUpdateAnnotations(timestamp_ns, values);
  }

  const orbit_client_protos::TimerInfo* GetLeft(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  const orbit_client_protos::TimerInfo* GetRight(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  const orbit_client_protos::TimerInfo* GetUp(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  const orbit_client_protos::TimerInfo* GetDown(
      const orbit_client_protos::TimerInfo& /*info*/) const override {
    return nullptr;
  }
  [[nodiscard]] uint64_t GetMinTime() const override;
  [[nodiscard]] uint64_t GetMaxTime() const override;

 protected:
  void DoUpdatePrimitives(PrimitiveAssembler& primitive_assembler, TextRenderer& text_renderer,
                          uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) override;

 private:
  void UpdatePositionOfSubtracks() override;

  std::shared_ptr<MajorPageFaultsTrack> major_page_faults_track_;
  std::shared_ptr<MinorPageFaultsTrack> minor_page_faults_track_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_PAGE_FAULTS_TRACK_H_
