// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "TimelineTicks.h"

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

  uint64_t scale = GetScale(end_ns + 1 - start_ns);

  uint64_t first_major_tick = ((start_ns + scale - 1) / scale) * scale;
  // TODO(b/208447247): Include MinorTicks.
  for (uint64_t major_tick = first_major_tick; major_tick <= end_ns; major_tick += scale) {
    ticks.push_back(std::make_pair(TickType::kMajorTick, major_tick));
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

int TimelineTicks::GetTimestampNumDigitsPrecision(uint64_t timestamp_ns) const {
  constexpr int kMaxDigitsPrecision = 9;  // 1ns = 0.000'000'001s

  uint64_t current_precision_ns = kNanosecondsPerSecond;
  for (int num_digits = 0; num_digits < kMaxDigitsPrecision;
       num_digits++, current_precision_ns /= 10) {
    if (timestamp_ns % current_precision_ns == 0) {
      return num_digits;
    }
  }
  return kMaxDigitsPrecision;
}

uint64_t TimelineTicks::GetScale(uint64_t visible_ns) const {
  // Biggest scale smaller than half the total range, as we want to see at least 2 major ticks.
  uint64_t half_visible_ns = visible_ns / 2;
  ORBIT_CHECK(half_visible_ns > 0);
  return *std::prev(kTimelineScales.upper_bound(half_visible_ns));
}

}  // namespace orbit_gl