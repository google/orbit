// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ScopeStats.h"

namespace orbit_client_data {
void ScopeStats::UpdateStats(uint64_t elapsed_nanos) {
  auto old_avg = static_cast<double>(ComputeAverageTimeNs());
  count_++;
  total_time_ns_ += elapsed_nanos;
  auto new_avg =
      static_cast<double>(ComputeAverageTimeNs());  // it should be double for arithmetics
  auto elapsed_nanos_double = static_cast<double>(elapsed_nanos);

  // variance(N) = ( (N-1)*variance(N-1) + (x-avg(N))*(x-avg(N-1)) ) / N
  variance_ns_ = ((static_cast<double>(count_ - 1) * variance_ns_ +
                   (elapsed_nanos_double - new_avg) * (elapsed_nanos_double - old_avg)) /
                  static_cast<double>(count_));

  if (max_ns_ < elapsed_nanos) {
    max_ns_ = elapsed_nanos;
  }

  if (min_ns_ == 0 || elapsed_nanos < min_ns_) {
    min_ns_ = elapsed_nanos;
  }
}

uint64_t ScopeStats::ComputeAverageTimeNs() const {
  if (count_ == 0) {
    return 0;
  }
  return total_time_ns_ / count_;
}
}  // namespace orbit_client_data