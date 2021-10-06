// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SIMPLE_TIMINGS_H_
#define ORBIT_GL_SIMPLE_TIMINGS_H_

#include <vector>

namespace orbit_gl {

class SimpleTimings {
 public:
  explicit SimpleTimings(size_t num_timings_to_store);

  double GetAverageTimeInSeconds();
  double GetMaxTimeInSeconds();
  double GetMinTimeInSeconds();

  void PushTiming(double time);
  void Reset();

 private:
  std::vector<double> recorded_timings_;
  size_t num_timings_to_store_ = 30;
  size_t timing_count_ = 0;

  double min_ = 0;
  double max_ = 0;
  double avg_ = 0;

  void UpdateCaches();
};

}  // namespace orbit_gl

#endif