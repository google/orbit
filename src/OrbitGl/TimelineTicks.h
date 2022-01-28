// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMELINE_IMPL_H_
#define ORBIT_GL_TIMELINE_IMPL_H_

#include <cstdint>
#include <set>
#include <vector>

namespace orbit_gl {

constexpr uint64_t kNanosecondsPerMicrosecond = 1000;
constexpr uint64_t kNanosecondsPerSecond = 1000 * 1000 * 1000;
constexpr uint64_t kNanosecondsPerMinute = 60 * kNanosecondsPerSecond;
constexpr uint64_t kNanosecondsPerHour = 60 * kNanosecondsPerMinute;

// TimelineTicks class manages the logic about the ticks, scale and visible timestamps in the
// timeline.
class TimelineTicks {
 public:
  enum class TickType { kMajorTick, kMinorTick };
  TimelineTicks() {}

  // For now, only major ticks.
  [[nodiscard]] std::vector<std::pair<TickType, uint64_t> > GetAllTicks(uint64_t start_ns,
                                                                        uint64_t end_ns) const;
  [[nodiscard]] std::vector<uint64_t> GetMajorTicks(uint64_t start_ns, uint64_t end_ns) const;
  // Number of digits needed to show precisely parts of a second in major-ticks' timestamps.
  [[nodiscard]] int GetTimestampsNumDigitsPrecision(uint64_t start_ns, uint64_t end_ns) const;

 private:
  uint64_t GetScale(uint64_t time_range_ns) const;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TIMELINE_IMPL_H_
