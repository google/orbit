// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <chrono>
#include <thread>

#include "OrbitBase/ThreadUtils.h"

TEST(ThreadUtils, GetCurrentThreadId) {
  orbit_base::thread_id_t current_tid = orbit_base::GetCurrentThreadId();
  orbit_base::thread_id_t worker_tid = 0;
  std::thread t([&worker_tid]() { worker_tid = orbit_base::GetCurrentThreadId(); });
  t.join();
  EXPECT_TRUE(worker_tid != 0);
  EXPECT_TRUE(worker_tid != current_tid);
}

TEST(ThreadUtils, GetSetThreadNames) {
  // Set thread name of exactly 15 characters. This should work on both Linux and Windows.
  const std::string kThreadName = "123456789012345";
  orbit_base::SetCurrentThreadName(kThreadName);
  std::string thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_EQ(kThreadName, thread_name);

  // On Linux, the maximum length for a thread name is 16 characters including '\0'.
  const size_t kMaxNonZeroCharactersLinux = 15;

  // Set thread name longer than 15 characters. The Linux version will truncate the name to 15
  // characters.
  const std::string kLongThreadName = "1234567890123456";
  EXPECT_GT(kLongThreadName.size(), kMaxNonZeroCharactersLinux);
  orbit_base::SetCurrentThreadName(kLongThreadName);
  std::string long_thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_EQ(long_thread_name.substr(0, kMaxNonZeroCharactersLinux),
            kLongThreadName.substr(0, kMaxNonZeroCharactersLinux));

#ifdef __linux
  // Test that the allowed thread name length hasn't increased on Linux.
  // If this fails, we should modify the Linux SetThreadName implementation
  // to allow for longer thread names.
  EXPECT_NE(pthread_setname_np(pthread_self(), kLongThreadName.data()), 0);
#endif
}
