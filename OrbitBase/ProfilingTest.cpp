// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
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

TEST(Profiling, GetSetThreadNames) {
  const std::string kThreadName = "ProfilingTest";
  SetThreadName(kThreadName);
  std::string thread_name = GetThreadName(GetCurrentThreadId());
  EXPECT_EQ(kThreadName, thread_name);

  // On Linux, the maximum length for a thread name is 16 characters including '\0'.
  const std::string kLongThreadName = "ProfilingTestVeryLongName";
  SetThreadName(kLongThreadName);
  std::string long_thread_name = GetThreadName(GetCurrentThreadId());
  EXPECT_THAT(kLongThreadName, testing::HasSubstr(long_thread_name.c_str()));
}
