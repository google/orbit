// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_base {

TEST(GetProcessIdsLinux, GetAllPids) {
  const auto pids = GetAllPids();

  // At least the test process needs to show up.
  ASSERT_FALSE(pids.empty());

  {
    const auto result = std::find(pids.begin(), pids.end(), getpid());
    ASSERT_NE(result, pids.end());
  }

  // We also assume PID 1 is always present.
  {
    const auto result = std::find(pids.begin(), pids.end(), 1);
    ASSERT_NE(result, pids.end());
  }
}

TEST(GetProcessIdsLinux, GetTidsOfProcess) {
  pid_t main_tid = syscall(SYS_gettid);
  pid_t thread_tid = -1;
  std::vector<pid_t> returned_tids{};

  std::mutex mutex;
  std::condition_variable cv_list;
  std::thread thread{[&] {
    thread_tid = syscall(SYS_gettid);
    {
      std::unique_lock<std::mutex> lock(mutex);
      while (returned_tids.empty()) {
        cv_list.wait(lock);
      }
    }
  }};

  {
    std::unique_lock<std::mutex> lock(mutex);
    returned_tids = GetTidsOfProcess(getpid());
    cv_list.notify_one();
  }
  thread.join();

  // There might be more than these two threads (i.e. when compiled with sanitizers enabled)
  EXPECT_THAT(returned_tids, ::testing::Contains(main_tid));
  EXPECT_THAT(returned_tids, ::testing::Contains(thread_tid));
}

TEST(GetProcessIdsLinux, GetTracerPidOfProcess) {
  pid_t current_pid = orbit_base::GetCurrentProcessIdNative();
  auto pid_or_error = orbit_base::GetTracerPidOfProcess(current_pid);
  EXPECT_FALSE(pid_or_error.has_error()) << pid_or_error.error().message();
  constexpr int kNoTracerPid = 0;
  EXPECT_EQ(pid_or_error.value(), kNoTracerPid);
}

}  // namespace orbit_base
