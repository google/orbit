// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SystemMemoryTrack.h"

#include <absl/strings/str_format.h>
#include <stdint.h>

#include <array>
#include <optional>

#include "DisplayFormats/DisplayFormats.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/CoreMath.h"

namespace orbit_gl {

namespace {
const std::string kTrackValueLabelUnit = "MB";
const std::array<std::string, kSystemMemoryTrackDimension> kSeriesName = {
    "Used", "Buffers / Cached", "Unused"};
constexpr uint64_t kMegabytesToBytes = 1024 * 1024;

}  // namespace

constexpr uint8_t kTrackValueDecimalDigits = 2;
SystemMemoryTrack::SystemMemoryTrack(CaptureViewElement* parent,
                                     const orbit_gl::TimelineInfoInterface* timeline_info,
                                     orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                     const orbit_client_data::ModuleManager* module_manager,
                                     const orbit_client_data::CaptureData* capture_data)
    : MemoryTrack<kSystemMemoryTrackDimension>(parent, timeline_info, viewport, layout, kSeriesName,
                                               kTrackValueDecimalDigits, kTrackValueLabelUnit,
                                               module_manager, capture_data) {
  // Colors are selected from https://convertingcolors.com/list/avery.html.
  // Use reddish colors for different used memories, yellowish colors for different cached memories
  // and greenish colors for different unused memories.
  const std::array<Color, kSystemMemoryTrackDimension> system_memory_track_colors{
      Color(231, 68, 53, 255),  // red
      Color(246, 196, 0, 255),  // orange
      Color(87, 166, 74, 255)   // green
  };
  SetSeriesColors(system_memory_track_colors);

  const std::string value_lower_bound_label = "Minimum: 0 GB";
  constexpr double kValueLowerBoundRawValue = 0.0;
  TrySetValueLowerBound(value_lower_bound_label, kValueLowerBoundRawValue);
}

std::string SystemMemoryTrack::GetName() const {
  return absl::StrFormat("Memory Usage: System (%s)", kTrackValueLabelUnit);
}

std::string SystemMemoryTrack::GetTooltip() const {
  return "Shows system-wide memory usage information.";
}

void SystemMemoryTrack::TrySetValueUpperBound(double total_mb) {
  const std::string value_upper_bound_label = "System Memory Total";
  std::string pretty_size =
      orbit_display_formats::GetDisplaySize(static_cast<uint64_t>(total_mb * kMegabytesToBytes));
  std::string pretty_label = absl::StrFormat("%s: %s", value_upper_bound_label, pretty_size);
  MemoryTrack<kSystemMemoryTrackDimension>::TrySetValueUpperBound(pretty_label, total_mb);
}

void SystemMemoryTrack::SetWarningThreshold(double warning_threshold_mb) {
  const std::string warning_threshold_label = "Production Limit";
  std::string pretty_size = orbit_display_formats::GetDisplaySize(
      static_cast<uint64_t>(warning_threshold_mb * kMegabytesToBytes));
  std::string pretty_label = absl::StrFormat("%s: %s", warning_threshold_label, pretty_size);
  MemoryTrack<kSystemMemoryTrackDimension>::SetWarningThreshold(pretty_label, warning_threshold_mb);
}

std::string SystemMemoryTrack::GetLegendTooltips(size_t legend_index) const {
  switch (static_cast<SeriesIndex>(legend_index)) {
    case SeriesIndex::kUsedMb:
      return "<b>Memory used by the system.</b><br/><br/>"
             "Derived from <i>MemTotal</i> - 'Unused' - 'Buffers / Cached', "
             "where <i>MemTotal</i> is a field in file <i>/proc/meminfo</i>.";
    case SeriesIndex::kBuffersOrCachedMb:
      return "<b>Memory in buffer cache or page cache.</b><br/><br/>"
             "Derived from <i>Buffers</i> + <i>Cached</i>, which are two fields in file "
             "<i>/proc/meminfo</i>.";
    case SeriesIndex::kUnusedMb:
      return "<b>Physical memory not used by the system</b><br/><br/>"
             "Derived from the <i>MemFree</i> field in file <i>/proc/meminfo</i>";
    default:
      ORBIT_UNREACHABLE();
  }
}

void SystemMemoryTrack::OnSystemMemoryInfo(
    const orbit_client_data::SystemMemoryInfo& system_memory_info) {
  if (system_memory_info.HasMissingInfo()) {
    return;
  }

  constexpr double kMegabytesToKilobytes = 1024.0;
  double total_mb =
      RoundPrecision(static_cast<double>(system_memory_info.total_kb) / kMegabytesToKilobytes);
  double free_mb =
      RoundPrecision(static_cast<double>(system_memory_info.free_kb) / kMegabytesToKilobytes);
  double buffers_or_cached_mb = RoundPrecision(
      static_cast<double>(system_memory_info.buffers_kb + system_memory_info.cached_kb) /
      kMegabytesToKilobytes);
  double used_mb = total_mb - free_mb - buffers_or_cached_mb;
  AddValues(system_memory_info.timestamp_ns, {used_mb, buffers_or_cached_mb, free_mb});

  if (!GetValueUpperBound().has_value()) TrySetValueUpperBound(total_mb);
}

}  // namespace orbit_gl
