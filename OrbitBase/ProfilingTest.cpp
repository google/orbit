// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "OrbitBase/Profiling.h"

TEST(Profiling, MonotonicClock) {
  uint64_t t0 = MonotonicTimestampNs();
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  uint64_t t1 = MonotonicTimestampNs();
  EXPECT_TRUE(t1 > t0);
}

TEST(Profiling, TicksToDuration) {
  uint64_t t0 = 0;
  uint64_t t1 = 1000;
  uint64_t t2 = 3000;
  EXPECT_TRUE(TicksToDuration(t0, t1) == absl::Microseconds(1));
  EXPECT_TRUE(TicksToDuration(t0, t2) == absl::Microseconds(3));
  EXPECT_TRUE(TicksToDuration(t1, t2) == absl::Microseconds(2));
}

TEST(Profiling, TicksToMicroseconds) {
  uint64_t t0 = 0;
  uint64_t t1 = 1000;
  uint64_t t2 = 3000;
  double dt0 = TicksToMicroseconds(t0, t1);
  double dt1 = TicksToMicroseconds(t1, t2);
  EXPECT_TRUE(dt1 > dt0);
  constexpr double kEpsilon = 0.001;
  EXPECT_TRUE(abs(dt0 - 1.0) < kEpsilon);
  EXPECT_TRUE(abs(dt1 - 2.0) < kEpsilon);
}

#ifdef __linux__
TEST(Profiling, ThreadId) {
  pid_t current_tid = GetCurrentThreadId();
  EXPECT_TRUE(current_tid >= 0);
  pid_t worker_tid = 0;
  std::thread t([&worker_tid]() { worker_tid = GetCurrentThreadId(); });
  t.join();
  EXPECT_TRUE(worker_tid != 0);
  EXPECT_TRUE(worker_tid != current_tid);
}
#endif
