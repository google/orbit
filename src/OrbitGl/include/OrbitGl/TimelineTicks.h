// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMELINE_IMPL_H_
#define ORBIT_GL_TIMELINE_IMPL_H_

#include <cstdint>
#include <optional>
#include <set>
#include <utility>
#include <vector>

namespace orbit_gl {

constexpr uint64_t kNanosecondsPerMicrosecond = 1000;
constexpr uint64_t kNanosecondsPerSecond = 1000 * 1000 * 1000;
constexpr uint64_t kNanosecondsPerMinute = 60 * kNanosecondsPerSecond;
constexpr uint64_t kNanosecondsPerHour = 60 * kNanosecondsPerMinute;
constexpr uint64_t kNanosecondsPerDay = 24 * kNanosecondsPerHour;
constexpr uint64_t kNanosecondsPerMonth = 30 * kNanosecondsPerDay;

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
  [[nodiscard]] std::optional<uint64_t> GetPreviousMajorTick(uint64_t start_ns,
                                                             uint64_t end_ns) const;
  // Number of digits needed to show precisely parts of a second in a timestamp.
  [[nodiscard]] static uint32_t GetTimestampNumDigitsPrecision(uint64_t timestamp_ns);

 private:
  [[nodiscard]] static uint64_t GetMajorTicksScale(uint64_t visible_ns);
  [[nodiscard]] uint64_t GetMinorTicksScale(uint64_t visible_ns) const;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TIMELINE_IMPL_H_
