// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "OrbitGl/TimelineTicks.h"

#include <iterator>
#include <optional>
#include <set>

#include "OrbitBase/Logging.h"

namespace orbit_gl {

// 10^x for each unit and middle points.
static std::set<uint64_t> GenerateScales() {
  std::set<uint64_t> scales_only_10_x;

  // Scales under 1 second.
  uint64_t scale_ns = 1;
  for (int i = 0; i < 9; i++) {
    scales_only_10_x.insert(scale_ns);
    scale_ns *= 10;
  }

  // Second and minute scales
  scales_only_10_x.insert(1 * kNanosecondsPerSecond);
  scales_only_10_x.insert(10 * kNanosecondsPerSecond);
  scales_only_10_x.insert(1 * kNanosecondsPerMinute);
  scales_only_10_x.insert(10 * kNanosecondsPerMinute);

  // Hour scales
  scale_ns = kNanosecondsPerHour;
  // Maximum scale: 1000 hours (more than a month).
  for (int i = 0; i < 4; i++) {
    scales_only_10_x.insert(scale_ns);
    scale_ns *= 10;
  }

  std::set<uint64_t> scales_including_middle_points;
  for (auto scale : scales_only_10_x) {
    if (scale > 1) {
      scales_including_middle_points.insert(scale / 2);
    }
    scales_including_middle_points.insert(scale);
  }

  return scales_including_middle_points;
}

const std::set<uint64_t> kTimelineScales = GenerateScales();

std::vector<std::pair<TimelineTicks::TickType, uint64_t> > TimelineTicks::GetAllTicks(
    uint64_t start_ns, uint64_t end_ns) const {
  std::vector<std::pair<TickType, uint64_t> > ticks;
  if (end_ns <= start_ns) return ticks;

  // We are including both borders (start_ns and end_ns) as visible points in time.
  uint64_t visible_ns = end_ns + 1 - start_ns;
  uint64_t major_scale = GetMajorTicksScale(visible_ns);
  uint64_t minor_scale = GetMinorTicksScale(visible_ns);

  uint64_t first_tick = ((start_ns + minor_scale - 1) / minor_scale) * minor_scale;
  for (uint64_t tick = first_tick; tick <= end_ns; tick += minor_scale) {
    ticks.push_back(std::make_pair(
        tick % major_scale == 0 ? TickType::kMajorTick : TickType::kMinorTick, tick));
  }
  return ticks;
}

std::vector<uint64_t> TimelineTicks::GetMajorTicks(uint64_t start_ns, uint64_t end_ns) const {
  std::vector<uint64_t> major_ticks;
  for (auto [tick_type, timestamp_ns] : GetAllTicks(start_ns, end_ns)) {
    if (tick_type == TickType::kMajorTick) {
      major_ticks.push_back(timestamp_ns);
    }
  }
  return major_ticks;
}

std::optional<uint64_t> TimelineTicks::GetPreviousMajorTick(uint64_t start_ns,
                                                            uint64_t end_ns) const {
  std::vector<uint64_t> major_ticks = GetMajorTicks(start_ns, end_ns);
  ORBIT_CHECK(major_ticks.size() != 0);

  uint64_t major_tick_scale = GetMajorTicksScale(end_ns + 1 - start_ns);
  if (major_ticks[0] < major_tick_scale) {
    return std::nullopt;
  }
  return major_ticks[0] - major_tick_scale;
}

uint32_t TimelineTicks::GetTimestampNumDigitsPrecision(uint64_t timestamp_ns) {
  constexpr uint32_t kMaxDigitsPrecision = 9;  // 1ns = 0.000'000'001s

  uint64_t current_precision_ns = kNanosecondsPerSecond;
  for (uint32_t num_digits = 0; num_digits < kMaxDigitsPrecision;
       num_digits++, current_precision_ns /= 10) {
    if (timestamp_ns % current_precision_ns == 0) {
      return num_digits;
    }
  }
  return kMaxDigitsPrecision;
}

uint64_t TimelineTicks::GetMinorTicksScale(uint64_t visible_ns) const {
  uint64_t major_scale = GetMajorTicksScale(visible_ns);
  // For consistency, minor ticks scale is the next finer scale after the one used for major ticks.
  if (major_scale <= 1) return 1;
  return *std::prev(kTimelineScales.lower_bound(major_scale));
}

uint64_t TimelineTicks::GetMajorTicksScale(uint64_t visible_ns) {
  // Biggest scale smaller than half the total range, as we want to see at least 2 major ticks.
  uint64_t half_visible_ns = visible_ns / 2;
  ORBIT_CHECK(half_visible_ns > 0);
  return *std::prev(kTimelineScales.upper_bound(half_visible_ns));
}

}  // namespace orbit_gl