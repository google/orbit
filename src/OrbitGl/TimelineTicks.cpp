// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "TimelineTicks.h"

#include "OrbitBase/Logging.h"

namespace orbit_gl {

TimelineTicks::TimelineTicks() {
  // We are inserting a set of possible scales.

  // Scales under 1 second.
  uint64_t scale_ns = 1;
  scales_.insert(scale_ns);
  for (int i = 0; i < 9; i++) {
    scale_ns *= 10;
    scales_.insert(scale_ns / 2);
    scales_.insert(scale_ns);
  }

  // Second and minute scales
  scales_.insert(5 * kSecondToNano);
  scales_.insert(10 * kSecondToNano);
  scales_.insert(15 * kSecondToNano);
  scales_.insert(30 * kSecondToNano);
  scales_.insert(1 * kMinuteToNano);
  scales_.insert(5 * kMinuteToNano);
  scales_.insert(10 * kMinuteToNano);
  scales_.insert(15 * kMinuteToNano);
  scales_.insert(30 * kMinuteToNano);

  // Hour scales
  scale_ns = kHourToNano;
  scales_.insert(scale_ns);
  // Maximum scale: 1000 hours (more than a month).
  for (int i = 0; i < 3; i++) {
    scale_ns *= 10;
    scales_.insert(scale_ns / 2);
    scales_.insert(scale_ns);
  }
}

std::vector<std::pair<TimelineTicks::TickType, uint64_t> > TimelineTicks::GetAllTicks(
    uint64_t start_ns, uint64_t end_ns) const {
  std::vector<std::pair<TickType, uint64_t> > ticks;
  uint64_t scale = GetScale(end_ns - start_ns);
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

uint64_t TimelineTicks::GetScale(uint64_t time_range_ns) const {
  // Biggest scale smaller than half the total range.
  uint64_t half_time_range_ns = time_range_ns / 2;
  CHECK(half_time_range_ns > 0);
  return *std::prev(scales_.upper_bound(time_range_ns / 2));
}

}  // namespace orbit_gl