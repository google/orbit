// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/MajorPageFaultsTrack.h"

#include <absl/strings/substitute.h>

#include <optional>
#include <utility>

#include "OrbitBase/Logging.h"

namespace orbit_gl {
MajorPageFaultsTrack::MajorPageFaultsTrack(Track* parent,
                                           const orbit_gl::TimelineInfoInterface* timeline_info,
                                           orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                           std::string cgroup_name,
                                           uint64_t memory_sampling_period_ms,
                                           const orbit_client_data::ModuleManager* module_manager,
                                           const orbit_client_data::CaptureData* capture_data)
    : BasicPageFaultsTrack(parent, timeline_info, viewport, layout, std::move(cgroup_name),
                           memory_sampling_period_ms, module_manager, capture_data) {
  index_of_series_to_highlight_ = static_cast<size_t>(SeriesIndex::kProcess);
}

std::string MajorPageFaultsTrack::GetTooltip() const {
  return "Shows major page faults statistics. A major page fault occurs when the requested page "
         "does not reside in the main memory or CPU cache, and has to be swapped from an external "
         "storage.<br/> The major page faults might cause slow performance of the target process.";
}

std::string MajorPageFaultsTrack::GetLegendTooltips(size_t legend_index) const {
  switch (static_cast<SeriesIndex>(legend_index)) {
    case SeriesIndex::kProcess:
      return absl::Substitute(
          "<b>Number of major page faults incurred by the $0 process during the sampling "
          "period ($2 ms).</b><br/><br/>"
          "Derived from the <i>majflt</i> field in file <i>/proc/$1/stat</i>.",
          capture_data_->process_name(), capture_data_->process_id(), memory_sampling_period_ms_);
    case SeriesIndex::kCGroup:
      return absl::Substitute(
          "<b>Number of major page faults incurred by the $0 cgroup during the sampling "
          "period ($1 ms).</b><br/><br/>"
          "Derived from the <i>pgmajfault</i> field in file "
          "<i>/sys/fs/cgroup/memory/$0/memory.stat</i>.",
          cgroup_name_, memory_sampling_period_ms_);
    case SeriesIndex::kSystem:
      return absl::Substitute(
          "<b>Number of system-wide major page faults occurred during the sampling "
          "period ($0 ms).</b><br/><br/>"
          "Derived from the <i>pgmajfault</i> field in file <i>/proc/vmstat</i>.",
          memory_sampling_period_ms_);
    default:
      ORBIT_UNREACHABLE();
  }
}

}  // namespace orbit_gl
