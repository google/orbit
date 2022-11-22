// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SimpleTimings.h"

#include <stddef.h>

#include <algorithm>
#include <limits>

namespace orbit_gl {
SimpleTimings::SimpleTimings(size_t num_timings_to_store)
    : num_timings_to_store_(num_timings_to_store) {
  recorded_timings_ms_.reserve(num_timings_to_store);
}

double SimpleTimings::GetAverageTimeMs() const { return avg_ms_; }

double SimpleTimings::GetMaxTimeMs() const { return max_ms_; }

double SimpleTimings::GetMinTimeMs() const { return min_ms_; }

void SimpleTimings::PushTimeMs(double time) {
  if (timing_count_ < num_timings_to_store_) {
    recorded_timings_ms_.push_back(time);
  } else {
    recorded_timings_ms_[timing_count_ % num_timings_to_store_] = time;
  }
  ++timing_count_;

  UpdateCaches();
}

void SimpleTimings::Reset() {
  timing_count_ = 0;
  recorded_timings_ms_.clear();
  UpdateCaches();
}

void SimpleTimings::UpdateCaches() {
  size_t count = recorded_timings_ms_.size();
  if (count == 0) {
    min_ms_ = max_ms_ = avg_ms_ = 0;
    return;
  }

  min_ms_ = std::numeric_limits<double>::max();
  max_ms_ = -min_ms_;
  double total = 0;
  for (size_t i = 0; i < count; ++i) {
    double value = recorded_timings_ms_[i];
    total += value;
    max_ms_ = std::max(max_ms_, value);
    min_ms_ = std::min(min_ms_, value);
  }

  avg_ms_ = total / count;
}

}  // namespace orbit_gl