// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/MinorPageFaultsTrack.h"

#include <absl/strings/substitute.h>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

std::string MinorPageFaultsTrack::GetTooltip() const {
  return "Shows minor page faults statistics. A minor page fault occurs when the requested page "
         "resides in main memory but the process cannot access it.";
}

std::string MinorPageFaultsTrack::GetLegendTooltips(size_t legend_index) const {
  switch (static_cast<SeriesIndex>(legend_index)) {
    case SeriesIndex::kProcess:
      return absl::Substitute(
          "<b>Number of minor page faults incurred by the $0 process during the sampling "
          "period ($2 ms).</b><br/><br/>"
          "Derived from the <i>minflt</i> field in file <i>/proc/$1/stat</i>.",
          capture_data_->process_name(), capture_data_->process_id(), memory_sampling_period_ms_);
    case SeriesIndex::kCGroup:
      return absl::Substitute(
          "<b>Number of minor page faults incurred by the $0 cgroup during the sampling "
          "period ($1 ms).</b><br/><br/>"
          "Derived from <i>pgfault - pgmajfault</i>, "
          "which are two fields in file <i>/sys/fs/cgroup/memory/$0/memory.stat</i>.",
          cgroup_name_, memory_sampling_period_ms_);
    case SeriesIndex::kSystem:
      return absl::Substitute(
          "<b>Number of system-wide minor page faults occurred during the sampling "
          "period ($0 ms).</b><br/><br/>"
          "Derived from <i>pgfault - pgmajfault</i>, which are two fields in file "
          "<i>/proc/vmstat</i>.",
          memory_sampling_period_ms_);
    default:
      ORBIT_UNREACHABLE();
  }
}

}  // namespace orbit_gl
