// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <chrono>
#include <memory>
#include <thread>

#include "OrbitBase/Profiling.h"

TEST(Profiling, MonotonicClock) {
  uint64_t t0 = orbit_base::CaptureTimestampNs();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  uint64_t t1 = orbit_base::CaptureTimestampNs();
  EXPECT_TRUE(t1 > t0);
}

TEST(Profiling, EstimatedClockResolutionNonNegative) {
  uint64_t resolution = orbit_base::EstimateClockResolution();

  // There's not really any guarantee we have for results of the above call.
  EXPECT_TRUE(resolution >= 0);
}
