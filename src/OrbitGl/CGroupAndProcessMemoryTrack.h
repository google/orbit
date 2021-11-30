// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CGROUP_AND_PROCESS_MEMORY_TRACK_H_
#define ORBIT_GL_CGROUP_AND_PROCESS_MEMORY_TRACK_H_

#include <string>
#include <utility>

#include "MemoryTrack.h"

namespace orbit_gl {

constexpr size_t kCGroupAndProcessMemoryTrackDimension = 4;

class CGroupAndProcessMemoryTrack final
    : public MemoryTrack<kCGroupAndProcessMemoryTrackDimension> {
 public:
  explicit CGroupAndProcessMemoryTrack(CaptureViewElement* parent,
                                       const orbit_gl::TimelineInfoInterface* timeline_info,
                                       orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                       const std::string& cgroup_name,
                                       const orbit_client_data::CaptureData* capture_data);

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetTooltip() const override;

  void TrySetValueUpperBound(double cgroup_limit_mb);

  void OnTimer(const orbit_client_protos::TimerInfo& timer_info) override;

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
