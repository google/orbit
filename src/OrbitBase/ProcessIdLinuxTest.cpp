// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <sys/types.h>
#include <syscall.h>

#include <condition_variable>
#include <filesystem>
#include <memory>
#include <mutex>
#include <thread>

#include "OrbitBase/ProcessId.h"
#include "OrbitBase/Result.h"
#include "absl/synchronization/mutex.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace orbit_base {

TEST(ProcessIdLinux, GetAllPids) {
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

TEST(ProcessIdLinux, GetTidsOfProcess) {
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

  std::vector<pid_t> expected_tids{main_tid, thread_tid};
  EXPECT_THAT(returned_tids, ::testing::UnorderedElementsAreArray(expected_tids));
}

TEST(ProcessIdLinux, GetAllTids) {
  pid_t main_tid = syscall(SYS_gettid);
  pid_t thread_tid = -1;
  std::vector<pid_t> returned_tids{};

  absl::Mutex mutex;
  std::thread thread{[&] {
    thread_tid = syscall(SYS_gettid);
    absl::MutexLock lock{&mutex};
    mutex.Await(absl::Condition(
        +[](std::vector<pid_t>* returned_tids) { return !returned_tids->empty(); },
        &returned_tids));
  }};

  {
    absl::MutexLock lock{&mutex};
    returned_tids = GetAllTids();
  }
  thread.join();

  std::vector<pid_t> expected_tids{1, main_tid, thread_tid};
  EXPECT_THAT(returned_tids, ::testing::IsSupersetOf(expected_tids));
}

}  // namespace orbit_base