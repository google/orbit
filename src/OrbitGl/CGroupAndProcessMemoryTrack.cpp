// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/CGroupAndProcessMemoryTrack.h"

#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>
#include <stdint.h>

#include <array>
#include <optional>
#include <string_view>
#include <utility>

#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

constexpr std::string_view kTrackValueLabelUnit = "MB";

[[nodiscard]] static std::array<std::string, kCGroupAndProcessMemoryTrackDimension>
CreateSeriesName(std::string_view cgroup_name, std::string_view process_name) {
  return {absl::StrFormat("Process [%s] RssAnon", process_name), "Other Processes RssAnon",
          absl::StrFormat("CGroup [%s] Mapped File", cgroup_name),
          absl::StrFormat("CGroup [%s] Unused", cgroup_name)};
}

static constexpr uint8_t kTrackValueDecimalDigits = 2;

CGroupAndProcessMemoryTrack::CGroupAndProcessMemoryTrack(
    CaptureViewElement* parent, const orbit_gl::TimelineInfoInterface* timeline_info,
    orbit_gl::Viewport* viewport, TimeGraphLayout* layout, std::string cgroup_name,
    const orbit_client_data::ModuleManager* module_manager,
    const orbit_client_data::CaptureData* capture_data)
    : MemoryTrack<kCGroupAndProcessMemoryTrackDimension>(
          parent, timeline_info, viewport, layout,
          CreateSeriesName(cgroup_name, capture_data->process_name()), kTrackValueDecimalDigits,
          std::string{kTrackValueLabelUnit}, module_manager, capture_data),
      cgroup_name_(std::move(cgroup_name)) {
  // Colors are selected from https://convertingcolors.com/list/avery.html.
  // Use reddish colors for different used memories, yellowish colors for different cached memories
  // and greenish colors for different unused memories.
  const std::array<Color, kCGroupAndProcessMemoryTrackDimension>
      c_group_and_process_memory_traccolors{
          Color(231, 68, 53, 255),    // red
          Color(185, 117, 181, 255),  // purple
          Color(246, 196, 0, 255),    // orange
          Color(87, 166, 74, 255)     // green
      };
  SetSeriesColors(c_group_and_process_memory_traccolors);

  const std::string value_lower_bound_label = "Minimum: 0 GB";
  constexpr double kValueLowerBoundRawValue = 0.0;
  TrySetValueLowerBound(value_lower_bound_label, kValueLowerBoundRawValue);
}

std::string CGroupAndProcessMemoryTrack::GetName() const {
  return absl::StrFormat("Memory Usage: CGroup (%s)", kTrackValueLabelUnit);
}

std::string CGroupAndProcessMemoryTrack::GetTooltip() const {
  return "Shows memory usage information for the target process and the memory cgroup it belongs "
         "to.<br/> The target process will be killed when the overall used memory approaches the "
         "cgroup limit.";
}

void CGroupAndProcessMemoryTrack::TrySetValueUpperBound(double cgroup_limit_mb) {
  const std::string value_upper_bound_label =
      absl::StrFormat("CGroup [%s] Memory Limit", cgroup_name_);
  constexpr uint64_t kMegabytesToBytes = 1024 * 1024;
  std::string pretty_size = orbit_display_formats::GetDisplaySize(
      static_cast<uint64_t>(cgroup_limit_mb * kMegabytesToBytes));
  std::string pretty_label = absl::StrFormat("%s: %s", value_upper_bound_label, pretty_size);
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
      ORBIT_UNREACHABLE();
  }
}

std::string CGroupAndProcessMemoryTrack::GetValueUpperBoundTooltip() const {
  // The developer instances have all of the same cgroup limits as the production instances, except
  // the game cgroup limit. More detailed information can be found in go/gamelet-ram-budget.
  std::string_view game_c_group_name = "user.slice/user-1000.slice/game";
  constexpr float kGameCGroupLimitGB = 7;

  if (cgroup_name_ != game_c_group_name) return "";
  return absl::StrFormat(
      "<b>The memory production limit of the %s cgroup is %.2fGB</b>.<br/><br/>"
      "<i>To launch game with the production cgroup limits, add the flag "
      "'--enforce-production-ram' to the 'ggp run' command</i>.",
      game_c_group_name, kGameCGroupLimitGB);
}

void CGroupAndProcessMemoryTrack::OnCgroupAndProcessMemoryInfo(
    const orbit_client_data::CgroupAndProcessMemoryInfo& cgroup_and_process_memory_info) {
  if (cgroup_and_process_memory_info.HasMissingInfo()) {
    return;
  }

  constexpr double kMegabytesToBytes = 1024.0 * 1024.0;
  constexpr double kMegabytesToKilobytes = 1024.0;
  double cgroup_limit_mb = RoundPrecision(
      static_cast<double>(cgroup_and_process_memory_info.cgroup_limit_bytes) / kMegabytesToBytes);
  double cgroup_rss_anon_mb = RoundPrecision(
      static_cast<double>(cgroup_and_process_memory_info.cgroup_rss_bytes) / kMegabytesToBytes);
  double cgroup_mapped_file_mb =
      RoundPrecision(static_cast<double>(cgroup_and_process_memory_info.cgroup_mapped_file_bytes) /
                     kMegabytesToBytes);
  double process_rss_anon_mb =
      RoundPrecision(static_cast<double>(cgroup_and_process_memory_info.process_rss_anon_kb) /
                     kMegabytesToKilobytes);
  double other_rss_anon_mb = cgroup_rss_anon_mb - process_rss_anon_mb;
  double unused_mb = cgroup_limit_mb - cgroup_rss_anon_mb - cgroup_mapped_file_mb;
  AddValues(cgroup_and_process_memory_info.timestamp_ns,
            {process_rss_anon_mb, other_rss_anon_mb, cgroup_mapped_file_mb, unused_mb});

  if (!GetValueUpperBound().has_value()) TrySetValueUpperBound(cgroup_limit_mb);
}

}  // namespace orbit_gl
