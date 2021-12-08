// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SIMPLE_TIMINGS_H_
#define ORBIT_GL_SIMPLE_TIMINGS_H_

#include <stddef.h>

#include <vector>
namespace orbit_gl {

class SimpleTimings {
 public:
  explicit SimpleTimings(size_t num_timings_to_store);

  [[nodiscard]] double GetAverageTimeMs() const;
  [[nodiscard]] double GetMaxTimeMs() const;
  [[nodiscard]] double GetMinTimeMs() const;

  void PushTimeMs(double time_in_ms);
  void Reset();

 private:
  std::vector<double> recorded_timings_ms_;
  size_t num_timings_to_store_ = 30;
  size_t timing_count_ = 0;

  double min_ms_ = 0;
  double max_ms_ = 0;
  double avg_ms_ = 0;

  void UpdateCaches();
};

}  // namespace orbit_gl

#endif