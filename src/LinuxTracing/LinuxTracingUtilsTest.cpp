// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/synchronization/mutex.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <pthread.h>
#include <sys/types.h>
#include <syscall.h>
#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include "LinuxTracingUtils.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_linux_tracing {

TEST(GetThreadName, LinuxTracingTests) {
  // Thread names have a length limit of 15 characters.
  std::string expected_name = std::string{"LinuxTracingTests"}.substr(0, 15);
  std::string returned_name = orbit_base::GetThreadName(getpid());
  EXPECT_EQ(returned_name, expected_name);
}

TEST(GetThreadState, LinuxTracingTestsMainAndAnother) {
  pid_t main_tid = syscall(SYS_gettid);

  std::optional<char> main_state_initial = GetThreadState(main_tid);
  ASSERT_TRUE(main_state_initial.has_value());
  EXPECT_EQ('R', main_state_initial.value());

  absl::Mutex mutex;
  pid_t thread_tid = -1;
  std::optional<char> thread_state_holding_mutex;
  std::optional<char> main_state_waiting_mutex;

  mutex.Lock();
  std::thread thread{[&] {
    // Make sure /proc/<pid>/stat is parsed correctly
    // even when the thread name contains spaces and parentheses.
    pthread_setname_np(pthread_self(), ") )  )()( )(  )");
    {
      absl::MutexLock lock{&mutex};
      thread_tid = syscall(SYS_gettid);
      thread_state_holding_mutex = GetThreadState(thread_tid);
      main_state_waiting_mutex = GetThreadState(main_tid);
    }
    // Let the main thread read this thread's state while this thread is in the sleep_for and verify
    // that in such a case the state is also 'S'.
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }};

  mutex.Await(absl::Condition(
      +[](pid_t* tid) { return *tid != -1; }, &thread_tid));
  mutex.Unlock();

  ASSERT_TRUE(thread_state_holding_mutex.has_value());
  EXPECT_EQ('R', thread_state_holding_mutex.value());
  ASSERT_TRUE(main_state_waiting_mutex.has_value());
  EXPECT_EQ('S', main_state_waiting_mutex.value());  // Interruptible sleep

  // Make sure `thread` has had the time to call sleep_for.
  std::this_thread::sleep_for(std::chrono::milliseconds(50));
  std::optional<char> thread_state_in_sleep_for = GetThreadState(thread_tid);
  ASSERT_TRUE(thread_state_in_sleep_for.has_value());
  EXPECT_EQ('S', thread_state_in_sleep_for.value());

  thread.join();
  // Make sure the kernel has had the time to clean up `thread` from the /proc filesystem.
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  std::optional<char> thread_state_exited = GetThreadState(thread_tid);
  EXPECT_FALSE(thread_state_exited.has_value());
}

TEST(ExtractCpusetFromCgroup, NoCpuset) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  ASSERT_FALSE(returned_cpuset.has_value());
}

TEST(ExtractCpusetFromCgroup, OnlyCpusetInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "8:cpuset:/groupname/foo\n"
      "6:cpu,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ExtractCpusetFromCgroup, CpusetLastInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuacct,cpuset:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ExtractCpusetFromCgroup, CpusetMiddleInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuset,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset = ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ParseCpusetCpus, Empty) {
  std::string cpuset_cpus_content = "";
  std::vector<int> returned_cpus = ParseCpusetCpus(cpuset_cpus_content);
  EXPECT_TRUE(returned_cpus.empty());
}

TEST(ParseCpusetCpus, SingleValuesAndRanges) {
  std::string cpuset_cpus_content = "0-2,4,7,12-14";
  std::vector<int> returned_cpus = ParseCpusetCpus(cpuset_cpus_content);
  EXPECT_THAT(returned_cpus, ::testing::ElementsAre(0, 1, 2, 4, 7, 12, 13, 14));
}

}  // namespace orbit_linux_tracing
