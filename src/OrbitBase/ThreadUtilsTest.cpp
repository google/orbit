// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/synchronization/mutex.h>
#include <gtest/gtest.h>
#include <limits.h>
#include <string.h>

#ifdef _WIN32
// No special windows header needed
#else
#include <pthread.h>
#endif

#include <algorithm>
#include <cstdint>
#include <string>
#include <thread>
#include <vector>

#include "OrbitBase/ThreadUtils.h"

TEST(ThreadUtils, GetCurrentThreadId) {
  uint32_t current_tid = orbit_base::GetCurrentThreadId();
  uint32_t worker_tid = 0;

  std::thread t([&worker_tid]() { worker_tid = orbit_base::GetCurrentThreadId(); });
  t.join();
  EXPECT_TRUE(worker_tid != 0);
  EXPECT_TRUE(worker_tid != current_tid);
}

TEST(ThreadUtils, GetSetCurrentThreadShortName) {
  // Set thread name of exactly 15 characters. This should work on both Linux and Windows.
  constexpr const char* kThreadName = "123456789012345";
  orbit_base::SetCurrentThreadName(kThreadName);
  std::string thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_EQ(kThreadName, thread_name);
}

TEST(ThreadUtils, GetSetCurrentThreadLongName) {
  // On Linux, the maximum length for a thread name is 16 characters including '\0'.
  static constexpr size_t kMaxNonZeroCharactersLinux = 15;

  // Set thread name longer than 15 characters. The Linux version will truncate the name to 15
  // characters.
  constexpr const char* kLongThreadName = "1234567890123456";
  EXPECT_GT(strlen(kLongThreadName), kMaxNonZeroCharactersLinux);
  orbit_base::SetCurrentThreadName(kLongThreadName);
  std::string long_thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_EQ(long_thread_name.substr(0, kMaxNonZeroCharactersLinux),
            std::string(kLongThreadName, kMaxNonZeroCharactersLinux));

#ifdef __linux
  // Test that the allowed thread name length hasn't increased on Linux.
  // If this fails, we should modify the Linux SetThreadName implementation
  // to allow for longer thread names.
  EXPECT_NE(pthread_setname_np(pthread_self(), kLongThreadName), 0);
#endif
}

TEST(ThreadUtils, GetSetCurrentThreadEmptyName) {
  // Set thread name of exactly 15 characters. This should work on both Linux and Windows.
  constexpr const char* kEmptyThreadName = "";
  orbit_base::SetCurrentThreadName(kEmptyThreadName);
  std::string thread_name = orbit_base::GetThreadName(orbit_base::GetCurrentThreadId());
  EXPECT_EQ(kEmptyThreadName, thread_name);
}

TEST(ThreadUtils, GetThreadName) {
  absl::Mutex mutex;
  uint32_t other_tid = 0;
  bool other_name_read = false;
  static constexpr const char* kThreadName = "OtherThread";

  std::thread other_thread{[&mutex, &other_tid, &other_name_read] {
    orbit_base::SetCurrentThreadName(kThreadName);
    {
      absl::MutexLock lock{&mutex};
      other_tid = orbit_base::GetCurrentThreadId();
    }
    {
      absl::MutexLock lock{&mutex};
      // Wait for the main thread to read this thread's name before exiting.
      mutex.Await(absl::Condition(
          +[](bool* other_name_read) { return *other_name_read; }, &other_name_read));
    }
  }};

  {
    absl::MutexLock lock{&mutex};
    // Wait for other_thread to set its own name and communicate its pid.
    mutex.Await(absl::Condition(
        +[](uint32_t* other_tid) { return *other_tid != 0; }, &other_tid));
  }
  std::string other_name = orbit_base::GetThreadName(other_tid);
  EXPECT_EQ(other_name, kThreadName);
  {
    absl::MutexLock lock{&mutex};
    other_name_read = true;
  }
  other_thread.join();
}

TEST(ThreadUtils, ValidIds) {
#ifdef __linux
  std::vector<int32_t> valid_native_thread_ids{0, 1, 2, 3, INT_MAX - 1, INT_MAX};
  std::vector<int32_t>& valid_native_process_ids = valid_native_thread_ids;
#else  // Windows
  std::vector<uint32_t> valid_native_thread_ids{4, 8, UINT_MAX - 7, UINT_MAX - 3};
  std::vector<uint32_t>& valid_native_process_ids = valid_native_thread_ids;
#endif

  for (auto native_tid : valid_native_thread_ids) {
    uint32_t tid = orbit_base::FromNativeThreadId(native_tid);
    EXPECT_TRUE(orbit_base::IsValidThreadId(tid)) << "tid == " << tid;
  }

  for (auto native_pid : valid_native_process_ids) {
    uint32_t pid = orbit_base::FromNativeProcessId(native_pid);
    EXPECT_TRUE(orbit_base::IsValidProcessId(pid)) << "pid == " << pid;
  }
}

TEST(ThreadUtils, InvalidIds) {
#ifdef __linux
  std::vector<int32_t> invalid_native_thread_ids{-INT_MAX, -2};
  const std::vector<int32_t>& invalid_native_process_ids = invalid_native_thread_ids;
#else  // Windows
  std::vector<uint32_t> invalid_native_thread_ids{1, 2, 3, 5};
  std::vector<uint32_t> invalid_native_process_ids = {1, 2, 3, 5};
#endif

  for (auto tid : invalid_native_thread_ids) {
    EXPECT_DEATH((void)orbit_base::FromNativeThreadId(tid), "Check failed") << "tid == " << tid;
  }

  for (auto pid : invalid_native_process_ids) {
    EXPECT_DEATH((void)orbit_base::FromNativeProcessId(pid), "Check failed") << "pid == " << pid;
  }
}
