// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "OrbitBase/ThreadUtils.h"

#ifdef __linux__
TEST(ThreadUtils, ThreadId) {
  uint32_t current_tid = orbit_base::GetCurrentThreadId();
  EXPECT_TRUE(current_tid >= 0);
  uint32_t worker_tid = 0;
  std::thread t([&worker_tid]() { worker_tid = orbit_base::GetCurrentThreadId(); });
  t.join();
  EXPECT_TRUE(worker_tid != 0);
  EXPECT_TRUE(worker_tid != current_tid);
}
#endif

TEST(ThreadUtils, GetSetThreadNames) {
  const std::string kThreadName = "ThreadUtilsTest";
  orbit_base::SetCurrentThreadName(kThreadName);
  std::string thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_EQ(kThreadName, thread_name);

  // On Linux, the maximum length for a thread name is 16 characters including '\0'.
  const size_t kMaxNonZeroCharactersLinux = 15;
  const std::string kLongThreadName = "ThreadUtilsTestVeryLongName";
  orbit_base::SetCurrentThreadName(kLongThreadName);
  std::string long_thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_GE(long_thread_name.size(), kMaxNonZeroCharactersLinux);
  EXPECT_THAT(kLongThreadName, testing::HasSubstr(long_thread_name.c_str()));
}
