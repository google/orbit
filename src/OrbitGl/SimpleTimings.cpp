// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SimpleTimings.h"

#include <algorithm>

namespace orbit_gl {
SimpleTimings::SimpleTimings(size_t num_timings_to_store)
    : num_timings_to_store_(num_timings_to_store) {
  recorded_timings_.reserve(num_timings_to_store);
}

double SimpleTimings::GetAverageTimeInSeconds() { return avg_; }

double SimpleTimings::GetMaxTimeInSeconds() { return max_; }

double SimpleTimings::GetMinTimeInSeconds() { return min_; }

void SimpleTimings::PushTiming(double time) {
  if (timing_count_ < num_timings_to_store_) {
    recorded_timings_.push_back(time);
  } else {
    recorded_timings_[timing_count_ % num_timings_to_store_] = time;
  }
  ++timing_count_;

  UpdateCaches();
}

void SimpleTimings::Reset() {
  timing_count_ = 0;
  recorded_timings_.clear();
  UpdateCaches();
}

void SimpleTimings::UpdateCaches() {
  size_t count = recorded_timings_.size();
  if (count == 0) {
    min_ = max_ = avg_ = 0;
    return;
  }

  min_ = std::numeric_limits<double>::max();
  max_ = -min_;
  double total = 0;
  for (size_t i = 0; i < count; ++i) {
    double value = recorded_timings_[i];
    total += value;
    max_ = std::max(max_, value);
    min_ = std::min(min_, value);
  }

  avg_ = total / count;
}

}  // namespace orbit_gl