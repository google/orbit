// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TIMELINE_IMPL_H_
#define ORBIT_GL_TIMELINE_IMPL_H_

#include <cstdint>
#include <set>
#include <vector>

namespace orbit_gl {

constexpr uint64_t kSecondToNano = 1000 * 1000 * 1000;
constexpr uint64_t kMinuteToNano = 60 * kSecondToNano;
constexpr uint64_t kHourToNano = 60 * kMinuteToNano;

class TimelineTicks {
  enum class TickType { MajorTick, MinorTick };

 public:
  TimelineTicks();

  // For now, only major ticks.
  [[nodiscard]] std::vector<std::pair<TickType, uint64_t> > GetTicks(uint64_t start_ns,
                                                                     uint64_t end_ns) const;
  [[nodiscard]] std::vector<uint64_t> GetMajorTicks(uint64_t start_ns, uint64_t end_ns) const;

 private:
  uint64_t GetScale(uint64_t time_range_ns) const;
  std::set<uint64_t> scales_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TIMELINE_IMPL_H_
