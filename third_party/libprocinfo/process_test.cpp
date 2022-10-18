/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <procinfo/process.h>

#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#include <chrono>
#include <set>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include <android-base/file.h>
#include <android-base/logging.h>
#include <android-base/stringprintf.h>
#include <android-base/unique_fd.h>

using namespace std::chrono_literals;

#if !defined(__BIONIC__)
#include <syscall.h>
static pid_t gettid() {
  return syscall(__NR_gettid);
}
#endif

TEST(process_info, process_info_smoke) {
  android::procinfo::ProcessInfo self;
  ASSERT_TRUE(android::procinfo::GetProcessInfo(gettid(), &self));
  ASSERT_EQ(gettid(), self.tid);
  ASSERT_EQ(getpid(), self.pid);
  ASSERT_EQ(getppid(), self.ppid);
  ASSERT_EQ(getuid(), self.uid);
  ASSERT_EQ(getgid(), self.gid);
}

TEST(process_info, process_info_proc_pid_fd_smoke) {
  android::procinfo::ProcessInfo self;
  int fd = open(android::base::StringPrintf("/proc/%d", gettid()).c_str(), O_DIRECTORY | O_RDONLY);
  ASSERT_NE(-1, fd);
  ASSERT_TRUE(android::procinfo::GetProcessInfoFromProcPidFd(fd, &self));

  // Process name is capped at 15 bytes.
  ASSERT_EQ("libprocinfo_tes", self.name);
  ASSERT_EQ(gettid(), self.tid);
  ASSERT_EQ(getpid(), self.pid);
  ASSERT_EQ(getppid(), self.ppid);
  ASSERT_EQ(getuid(), self.uid);
  ASSERT_EQ(getgid(), self.gid);
  close(fd);
}

TEST(process_info, process_tids_smoke) {
  pid_t main_tid = gettid();
  std::thread([main_tid]() {
    pid_t thread_tid = gettid();

    {
      std::vector<pid_t> vec;
      ASSERT_TRUE(android::procinfo::GetProcessTids(getpid(), &vec));
      ASSERT_EQ(1, std::count(vec.begin(), vec.end(), main_tid));
      ASSERT_EQ(1, std::count(vec.begin(), vec.end(), thread_tid));
    }

    {
      std::set<pid_t> set;
      ASSERT_TRUE(android::procinfo::GetProcessTids(getpid(), &set));
      ASSERT_EQ(1, std::count(set.begin(), set.end(), main_tid));
      ASSERT_EQ(1, std::count(set.begin(), set.end(), thread_tid));
    }
  }).join();
}

TEST(process_info, process_state) {
  int pipefd[2];
  ASSERT_EQ(0, pipe2(pipefd, O_CLOEXEC));
  pid_t forkpid = fork();

  ASSERT_NE(-1, forkpid);
  if (forkpid == 0) {
    close(pipefd[1]);
    char buf;
    TEMP_FAILURE_RETRY(read(pipefd[0], &buf, 1));
    _exit(0);
  }

  // Give the child some time to get to the read.
  std::this_thread::sleep_for(100ms);

  android::procinfo::ProcessInfo procinfo;
  ASSERT_TRUE(android::procinfo::GetProcessInfo(forkpid, &procinfo));
  ASSERT_EQ(android::procinfo::kProcessStateSleeping, procinfo.state);

  ASSERT_EQ(0, kill(forkpid, SIGKILL));

  // Give the kernel some time to kill the child.
  std::this_thread::sleep_for(100ms);

  ASSERT_TRUE(android::procinfo::GetProcessInfo(forkpid, &procinfo));
  ASSERT_EQ(android::procinfo::kProcessStateZombie, procinfo.state);

  ASSERT_EQ(forkpid, waitpid(forkpid, nullptr, 0));
}

static uint64_t read_uptime_secs() {
  std::string uptime;
  if (!android::base::ReadFileToString("/proc/uptime", &uptime)) {
    PLOG(FATAL) << "failed to read /proc/uptime";
  }
  return strtoll(uptime.c_str(), nullptr, 10);
}

TEST(process_info, process_start_time) {
  uint64_t start = read_uptime_secs();
  int pipefd[2];
  ASSERT_EQ(0, pipe2(pipefd, O_CLOEXEC));

  std::this_thread::sleep_for(1000ms);

  pid_t forkpid = fork();

  ASSERT_NE(-1, forkpid);
  if (forkpid == 0) {
    close(pipefd[1]);
    char buf;
    TEMP_FAILURE_RETRY(read(pipefd[0], &buf, 1));
    _exit(0);
  }

  std::this_thread::sleep_for(1000ms);

  uint64_t end = read_uptime_secs();

  android::procinfo::ProcessInfo procinfo;
  ASSERT_TRUE(android::procinfo::GetProcessInfo(forkpid, &procinfo));

  // starttime is measured in clock ticks: uptime is in seconds:
  uint64_t process_start = procinfo.starttime / sysconf(_SC_CLK_TCK);
  ASSERT_LE(start, process_start);
  ASSERT_LE(process_start, end);

  ASSERT_EQ(0, kill(forkpid, SIGKILL));
  ASSERT_EQ(forkpid, waitpid(forkpid, nullptr, 0));
}

TEST(process_info, GetProcessInfoFromProcPidFd_set_error) {
  TemporaryDir tmp_dir;

  android::base::unique_fd dirfd(open(tmp_dir.path, O_DIRECTORY | O_RDONLY));
  android::procinfo::ProcessInfo procinfo;
  std::string error;

  // failed to open status file error
  // No segfault if not given error string.
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo));
  // Set error when given error string.
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo, &error));
  ASSERT_EQ(error, "failed to open status fd in GetProcessInfoFromProcPidFd");

  // failed to parse status file error
  std::string status_file = std::string(tmp_dir.path) + "/status";
  ASSERT_TRUE(android::base::WriteStringToFile("invalid data", status_file));
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo));
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo, &error));
  ASSERT_EQ(error, "failed to parse /proc/<pid>/status");

  // failed to read stat file error
  ASSERT_TRUE(android::base::WriteStringToFile(
      "Name:\tsh\nTgid:\t0\nPid:\t0\nTracerPid:\t0\nUid:\t0\nGid:\t0\n", status_file));
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo));
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo, &error));
  ASSERT_EQ(error, "failed to read /proc/<pid>/stat");

  // failed to parse stat file error
  std::string stat_file = std::string(tmp_dir.path) + "/stat";
  ASSERT_TRUE(android::base::WriteStringToFile("2027 (sh) invalid data", stat_file));
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo));
  ASSERT_FALSE(android::procinfo::GetProcessInfoFromProcPidFd(dirfd.get(), &procinfo, &error));
  ASSERT_EQ(error, "failed to parse /proc/<pid>/stat");
}
