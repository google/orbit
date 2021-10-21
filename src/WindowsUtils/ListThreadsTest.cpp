// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <windows.h>

// clang-format off
#include <libloaderapi.h>
// clang-format on

#include "OrbitBase/ThreadUtils.h"
#include "WindowsUtils/ListThreads.h"

namespace orbit_windows_utils {

TEST(ListThreads, ListThreadsContainsCurrentThread) {
  uint32_t pid = orbit_base::GetCurrentProcessId();
  uint32_t tid = orbit_base::GetCurrentThreadId();
  constexpr const char* kThreadName = "WindowsUtilsListThreads";
  orbit_base::SetCurrentThreadName(kThreadName);

  std::vector<Thread> threads = ListThreads(pid);
  EXPECT_NE(threads.size(), 0);

  std::string this_thread_name;
  for (const Thread& thread : threads) {
    if (thread.tid == tid) {
      this_thread_name = thread.name;
      break;
    }
  }

  EXPECT_FALSE(this_thread_name.empty());
  EXPECT_STREQ(this_thread_name.c_str(), kThreadName);
}

TEST(WindowsUtils, ListAllThreadsContainsCurrentThread) {
  uint32_t tid = orbit_base::GetCurrentThreadId();
  constexpr const char* kThreadName = "WindowsUtilsListAllThreads";
  orbit_base::SetCurrentThreadName(kThreadName);

  std::vector<Thread> threads = ListAllThreads();
  EXPECT_NE(threads.size(), 0);

  std::string this_thread_name;
  for (const Thread& thread : threads) {
    if (thread.tid == tid) {
      this_thread_name = thread.name;
      break;
    }
  }

  EXPECT_FALSE(this_thread_name.empty());
  EXPECT_STREQ(this_thread_name.c_str(), kThreadName);
}

}  // namespace orbit_windows_utils
