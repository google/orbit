// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MAJOR_PAGE_FAULTS_TRACK_H_
#define ORBIT_GL_MAJOR_PAGE_FAULTS_TRACK_H_

#include <stddef.h>
#include <stdint.h>

#include <string>
#include <utility>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "OrbitGl/BasicPageFaultsTrack.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

class MajorPageFaultsTrack final : public BasicPageFaultsTrack {
 public:
  explicit MajorPageFaultsTrack(Track* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
                                orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                std::string cgroup_name, uint64_t memory_sampling_period_ms,
                                const orbit_client_data::ModuleManager* module_manager,
                                const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override { return "Page Faults: Major"; }
  [[nodiscard]] std::string GetTooltip() const override;

 private:
  [[nodiscard]] std::string GetLegendTooltips(size_t legend_index) const override;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MAJOR_PAGE_FAULTS_TRACK_H_
