// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CGroupAndProcessMemoryTrack.h"

#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>

#include "DisplayFormats/DisplayFormats.h"

namespace orbit_gl {

namespace {

const std::string kTrackValueLabelUnit = "MB";
const std::string kTrackName = absl::StrFormat("CGroup Memory Usage (%s)", kTrackValueLabelUnit);

static std::array<std::string, kCGroupAndProcessMemoryTrackDimension> CreateSeriesName(
    const std::string& cgroup_name, const std::string& process_name) {
  return {absl::StrFormat("Process [%s] RssAnon", process_name), "Other Processes RssAnon",
          absl::StrFormat("CGroup [%s] Mapped File", cgroup_name),
          absl::StrFormat("CGroup [%s] Unused", cgroup_name)};
}

}  // namespace

CGroupAndProcessMemoryTrack::CGroupAndProcessMemoryTrack(
    CaptureViewElement* parent, TimeGraph* time_graph, orbit_gl::Viewport* viewport,
    TimeGraphLayout* layout, const std::string& cgroup_name,
    const orbit_client_model::CaptureData* capture_data)
    : MemoryTrack<kCGroupAndProcessMemoryTrackDimension>(
          parent, time_graph, viewport, layout, kTrackName,
          CreateSeriesName(cgroup_name, capture_data->process_name()), capture_data),
      cgroup_name_(cgroup_name) {
  SetLabelUnit(kTrackValueLabelUnit);

  constexpr uint8_t kTrackValueDecimalDigits = 2;
  SetNumberOfDecimalDigits(kTrackValueDecimalDigits);

  // Colors are selected from https://convertingcolors.com/list/avery.html.
  // Use reddish colors for different used memories, yellowish colors for different cached memories
  // and greenish colors for different unused memories.
  const std::array<Color, kCGroupAndProcessMemoryTrackDimension> kCGroupAndProcessMemoryTrackColors{
      Color(231, 68, 53, 255),   // red
      Color(249, 96, 111, 255),  // warm red
      Color(246, 196, 0, 255),   // orange
      Color(87, 166, 74, 255)    // green
  };
  SetSeriesColors(kCGroupAndProcessMemoryTrackColors);

  const std::string kValueLowerBoundLabel = "Minimum: 0 GB";
  constexpr double kValueLowerBoundRawValue = 0.0;
  TrySetValueLowerBound(kValueLowerBoundLabel, kValueLowerBoundRawValue);
}

std::string CGroupAndProcessMemoryTrack::GetTooltip() const {
  return "Shows memory usage information for the target process and the memory cgroup it belongs "
         "to. The target process will be killed when the overall used memory approaches the cgroup "
         "limit.";
}

void CGroupAndProcessMemoryTrack::TrySetValueUpperBound(double cgroup_limit_mb) {
  const std::string kValueUpperBoundLabel =
      absl::StrFormat("CGroup [%s] Memory Limit", cgroup_name_);
  constexpr uint64_t kMegabytesToBytes = 1024 * 1024;
  std::string pretty_size = orbit_display_formats::GetDisplaySize(
      static_cast<uint64_t>(cgroup_limit_mb * kMegabytesToBytes));
  std::string pretty_label = absl::StrFormat("%s: %s", kValueUpperBoundLabel, pretty_size);
  MemoryTrack<kCGroupAndProcessMemoryTrackDimension>::TrySetValueUpperBound(pretty_label,
                                                                            cgroup_limit_mb);
}

std::string CGroupAndProcessMemoryTrack::GetLegendTooltips(size_t legend_index) const {
  switch (static_cast<SeriesIndex>(legend_index)) {
    case SeriesIndex::kProcessRssAnonMb:
      return absl::Substitute(
          "<b>Resident anonymous memory of the target process $0.</b><br/><br/>"
          "Derived from the <i>RssAnon</i> field in file <i>/proc/$1/status</i>",
          capture_data_->process_name(), capture_data_->process_id());
    case SeriesIndex::kOtherRssAnonMb:
      return absl::Substitute(
          "<b>Resident anonymous memory of other processes in the $0 cgroup.</b><br/><br/>"
          "Derived from the cgroup anonymous memory - 'Process [$1] RssAnon',<br/>"
          "where the cgroup anonymous memory is extracted from the <i>rss</i> field in file "
          "<i>/sys/fs/cgroup/memory/$0/memory.stat</i>",
          cgroup_name_, capture_data_->process_name());
    case SeriesIndex::kCGroupMappedFileMb:
      return absl::Substitute(
          "<b>Resident file mapping of the $0 cgroup.</b><br/><br/>"
          "Derived from the <i>mapped_file</i> field in file<br/>"
          "<i>/sys/fs/cgroup/memory/$0/memory.stat</i>",
          cgroup_name_);
    case SeriesIndex::kUnusedMb:
      return absl::Substitute(
          "<b>Unused memory in the $0 cgroup.</b><br/><br/>"
          "Derived from cgroup memory limit - cgroup rss - cgroup mapped_file.<br/> "
          "The cgroup memory limit is extracted from file "
          "<i>/sys/fs/cgroup/memory/$0/memory.limit_in_bytes</i>",
          cgroup_name_);
    default:
      UNREACHABLE();
  }
}

std::string CGroupAndProcessMemoryTrack::GetValueUpperBoundTooltip() const {
  const std::string kGameCGroupName = "game";
  constexpr float kGameCGroupLimitGB = 7;

  if (cgroup_name_ != kGameCGroupName) return "";
  return absl::StrFormat(
      "<b>The memory production limit of the %s cgroup is %.2fGB</b>.<br/><br/>"
      "<i>To launch game with the production cgroup limits, add the flag "
      "'--enforce-production-ram' to the 'ggp run' command</i>.",
      kGameCGroupName, kGameCGroupLimitGB);
}
}  // namespace orbit_gl
