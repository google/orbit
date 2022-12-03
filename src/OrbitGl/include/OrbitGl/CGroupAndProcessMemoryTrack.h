// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CGROUP_AND_PROCESS_MEMORY_TRACK_H_
#define ORBIT_GL_CGROUP_AND_PROCESS_MEMORY_TRACK_H_

#include <stddef.h>

#include <string>
#include <utility>

#include "ClientData/CaptureData.h"
#include "ClientData/CgroupAndProcessMemoryInfo.h"
#include "ClientData/ModuleManager.h"
#include "OrbitGl/CaptureViewElement.h"
#include "OrbitGl/MemoryTrack.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/Viewport.h"

namespace orbit_gl {

constexpr size_t kCGroupAndProcessMemoryTrackDimension = 4;

class CGroupAndProcessMemoryTrack final
    : public MemoryTrack<kCGroupAndProcessMemoryTrackDimension> {
 public:
  explicit CGroupAndProcessMemoryTrack(CaptureViewElement* parent,
                                       const orbit_gl::TimelineInfoInterface* timeline_info,
                                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                       std::string group_name,
                                       const orbit_client_data::ModuleManager* module_manager,
                                       const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetTooltip() const override;

  void TrySetValueUpperBound(double cgroup_limit_mb);

  void OnCgroupAndProcessMemoryInfo(
      const orbit_client_data::CgroupAndProcessMemoryInfo& cgroup_and_process_memory_info);

  enum class SeriesIndex {
    kProcessRssAnonMb = 0,
    kOtherRssAnonMb = 1,
    kCGroupMappedFileMb = 2,
    kUnusedMb = 3
  };

 private:
  [[nodiscard]] std::string GetLegendTooltips(size_t legend_index) const override;
  [[nodiscard]] std::string GetValueUpperBoundTooltip() const override;
  std::string cgroup_name_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_CGROUP_AND_PROCESS_MEMORY_TRACK_H_
