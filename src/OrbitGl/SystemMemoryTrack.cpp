// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SystemMemoryTrack.h"

#include <absl/strings/str_format.h>

#include "ApiUtils/EncodedEvent.h"
#include "CaptureClient/CaptureEventProcessor.h"
#include "DisplayFormats/DisplayFormats.h"
#include "GrpcProtos/Constants.h"

namespace orbit_gl {

namespace {

using orbit_capture_client::CaptureEventProcessor;
using orbit_grpc_protos::kMissingInfo;

const std::string kTrackValueLabelUnit = "MB";
const std::array<std::string, kSystemMemoryTrackDimension> kSeriesName = {
    "Used", "Buffers / Cached", "Unused"};
constexpr uint64_t kMegabytesToBytes = 1024 * 1024;

}  // namespace

constexpr uint8_t kTrackValueDecimalDigits = 2;
SystemMemoryTrack::SystemMemoryTrack(CaptureViewElement* parent,
                                     const orbit_gl::TimelineInfoInterface* timeline_info,
                                     orbit_gl::Viewport* viewport, TimeGraphLayout* layout,
                                     const orbit_client_data::CaptureData* capture_data)
    : MemoryTrack<kSystemMemoryTrackDimension>(parent, timeline_info, viewport, layout, kSeriesName,
                                               kTrackValueDecimalDigits, kTrackValueLabelUnit,
                                               capture_data) {
  // Colors are selected from https://convertingcolors.com/list/avery.html.
  // Use reddish colors for different used memories, yellowish colors for different cached memories
  // and greenish colors for different unused memories.
  const std::array<Color, kSystemMemoryTrackDimension> kSystemMemoryTrackColors{
      Color(231, 68, 53, 255),  // red
      Color(246, 196, 0, 255),  // orange
      Color(87, 166, 74, 255)   // green
  };
  SetSeriesColors(kSystemMemoryTrackColors);

  const std::string kValueLowerBoundLabel = "Minimum: 0 GB";
  constexpr double kValueLowerBoundRawValue = 0.0;
  TrySetValueLowerBound(kValueLowerBoundLabel, kValueLowerBoundRawValue);
}

std::string SystemMemoryTrack::GetName() const {
  return absl::StrFormat("Memory Usage: System (%s)", kTrackValueLabelUnit);
}

std::string SystemMemoryTrack::GetTooltip() const {
  return "Shows system-wide memory usage information.";
}

void SystemMemoryTrack::TrySetValueUpperBound(double total_mb) {
  const std::string kValueUpperBoundLabel = "System Memory Total";
  std::string pretty_size =
      orbit_display_formats::GetDisplaySize(static_cast<uint64_t>(total_mb * kMegabytesToBytes));
  std::string pretty_label = absl::StrFormat("%s: %s", kValueUpperBoundLabel, pretty_size);
  MemoryTrack<kSystemMemoryTrackDimension>::TrySetValueUpperBound(pretty_label, total_mb);
}

void SystemMemoryTrack::SetWarningThreshold(double warning_threshold_mb) {
  const std::string kWarningThresholdLabel = "Production Limit";
  std::string pretty_size = orbit_display_formats::GetDisplaySize(
      static_cast<uint64_t>(warning_threshold_mb * kMegabytesToBytes));
  std::string pretty_label = absl::StrFormat("%s: %s", kWarningThresholdLabel, pretty_size);
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
      UNREACHABLE();
  }
}

void SystemMemoryTrack::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  int64_t total_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::SystemMemoryUsageEncodingIndex::kTotalKb)));
  int64_t unused_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::SystemMemoryUsageEncodingIndex::kFreeKb)));
  int64_t buffers_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::SystemMemoryUsageEncodingIndex::kBuffersKb)));
  int64_t cached_kb = orbit_api::Decode<int64_t>(timer_info.registers(
      static_cast<size_t>(CaptureEventProcessor::SystemMemoryUsageEncodingIndex::kCachedKb)));
  if (total_kb == kMissingInfo || unused_kb == kMissingInfo || buffers_kb == kMissingInfo ||
      cached_kb == kMissingInfo) {
    return;
  }

  constexpr double kMegabytesToKilobytes = 1024.0;
  double total_mb = RoundPrecision(static_cast<double>(total_kb) / kMegabytesToKilobytes);
  double unused_mb = RoundPrecision(static_cast<double>(unused_kb) / kMegabytesToKilobytes);
  double buffers_or_cached_mb =
      RoundPrecision(static_cast<double>(buffers_kb + cached_kb) / kMegabytesToKilobytes);
  double used_mb = total_mb - unused_mb - buffers_or_cached_mb;
  AddValues(timer_info.start(), {used_mb, buffers_or_cached_mb, unused_mb});

  if (!GetValueUpperBound().has_value()) TrySetValueUpperBound(total_mb);

  MemoryTrack<kSystemMemoryTrackDimension>::OnTimer(timer_info);
}

}  // namespace orbit_gl
