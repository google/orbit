// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>
#include <sys/syscall.h>

#include <condition_variable>
#include <mutex>
#include <thread>

#include "Utils.h"

namespace LinuxTracing {

TEST(ReadFile, ProcPidCommOfOrbitLinuxTracingTests) {
  std::string filename = absl::StrFormat("/proc/%d/comm", getpid());
  std::optional<std::string> returned_comm = ReadFile(filename);
  // Comm values have a size limit of 15 characters.
  std::string expected_comm =
      std::string{"OrbitLinuxTracingTests"}.substr(0, 15).append("\n");
  ASSERT_TRUE(returned_comm.has_value());
  EXPECT_EQ(returned_comm.value(), expected_comm);
}

TEST(ExecuteCommand, EchoHelloWorld) {
  std::string string_to_echo = "Hello, World!";
  std::optional<std::string> returned_result =
      ExecuteCommand(absl::StrFormat("echo %s", string_to_echo));
  std::string expected_result = string_to_echo + "\n";
  ASSERT_TRUE(returned_result.has_value());
  EXPECT_EQ(returned_result.value(), expected_result);
}

TEST(ListThreads, OrbitLinuxTracingTestsMainAndAnother) {
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
    returned_tids = ListThreads(getpid());
    cv_list.notify_one();
  }
  thread.join();

  std::vector<pid_t> expected_tids{main_tid, thread_tid};
  std::sort(expected_tids.begin(), expected_tids.end());
  EXPECT_THAT(returned_tids, ::testing::ElementsAreArray(expected_tids));
}

TEST(GetThreadName, OrbitLinuxTracingTests) {
  // Thread names have a length limit of 15 characters.
  std::string expected_name =
      std::string{"OrbitLinuxTracingTests"}.substr(0, 15);
  std::string returned_name = GetThreadName(getpid());
  EXPECT_EQ(returned_name, expected_name);
}

TEST(ExtractCpusetFromCgroup, NoCpuset) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset =
      ExtractCpusetFromCgroup(cgroup_content);
  ASSERT_FALSE(returned_cpuset.has_value());
}

TEST(ExtractCpusetFromCgroup, OnlyCpusetInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "8:cpuset:/groupname/foo\n"
      "6:cpu,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset =
      ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ExtractCpusetFromCgroup, CpusetLastInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuacct,cpuset:/groupname/foo";
  std::optional<std::string> returned_cpuset =
      ExtractCpusetFromCgroup(cgroup_content);
  std::string expected_cpuset = "/groupname/foo";
  ASSERT_TRUE(returned_cpuset.has_value());
  EXPECT_EQ(returned_cpuset.value(), expected_cpuset);
}

TEST(ExtractCpusetFromCgroup, CpusetMiddleInLine) {
  std::string cgroup_content =
      "11:memory:/groupname/foo\n"
      "6:cpu,cpuset,cpuacct:/groupname/foo";
  std::optional<std::string> returned_cpuset =
      ExtractCpusetFromCgroup(cgroup_content);
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

}  // namespace LinuxTracing
