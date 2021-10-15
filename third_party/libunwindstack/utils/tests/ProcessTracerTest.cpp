/*
 * Copyright (C) 2021 The Android Open Source Project
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

#include <dlfcn.h>
#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <system_error>
#include <thread>

#include <android-base/file.h>
#include <procinfo/process.h>

#include "OfflineUnwindUtils.h"
#include "ProcessTracer.h"
#include "tests/TestUtils.h"

namespace unwindstack {
namespace {

class ProcessTracerTest : public ::testing::TestWithParam<bool> {
 protected:
  enum class BoolOrTimeout {
    kSuccess = 0,
    kFail,
    kTimeout,
  };

  // Setup a child process that has a few threads that simply busy wait.
  void SetUp() override {
    // Setup signal handlers for child to let parent know that it is ready and for parent
    // to kill the child.
    child_is_ready_ = false;
    ASSERT_NE(SIG_ERR, signal(kChildIsReadySignal, [](int) { child_is_ready_ = true; }))
        << "Failed to set up signal handler for kChildIsReadySignal: " << strerror(errno);
    child_keep_running_ = true;
    ASSERT_NE(SIG_ERR, signal(kStopChildSignal, [](int) { child_keep_running_ = false; }))
        << "Failed to set up signal handler for kStopChildSignal: " << strerror(errno);

    pid_t parent_pid = getpid();
    child_pid_ = fork();
    if (child_pid_ == static_cast<pid_t>(-1)) FAIL() << "SetUp: fork() failed: " << strerror(errno);
    if (child_pid_ == 0) {
      ASSERT_NO_FATAL_FAILURE(ChildProcSpin(parent_pid));
    }

    // Make sure the child process has set up its threads before running the test.
    sigset_t signal_mask, old_mask;
    sigemptyset(&signal_mask);
    sigaddset(&signal_mask, kChildIsReadySignal);
    sigprocmask(SIG_BLOCK, &signal_mask, &old_mask);
    while (!child_is_ready_) sigsuspend(&old_mask);
    sigprocmask(SIG_UNBLOCK, &signal_mask, NULL);
  }

  void TearDown() override {
    // Send signal to join threads and exit.
    if (-1 == kill(child_pid_, kStopChildSignal)) {
      std::cerr << "TearDown: kill sending kStopChildSignal failed: " << strerror(errno) << ".\n";
      kill(child_pid_, SIGKILL);
    }
  }

  void ChildProcSpin(pid_t parent_pid) {
    // Busy wait in a dlopened local library so we can reliably test (across different
    // architecture) if a process is within a desired ELF.
    std::unique_ptr<void, decltype(&dlclose)> test_lib_handle(GetTestLibHandle(), &dlclose);
    ASSERT_TRUE(test_lib_handle);
    int (*busy_wait_func)() = reinterpret_cast<int (*)()>(dlsym(test_lib_handle.get(), "BusyWait"));
    ASSERT_NE(nullptr, busy_wait_func);

    std::array<std::thread, kNumThreads> threads;
    std::array<std::atomic_bool, kNumThreads> threads_are_ready{false, false, false, false, false};
    for (size_t i = 0; i < kNumThreads; ++i) {
      threads.at(i) = std::thread([&threads_are_ready, i, &busy_wait_func]() {
        while (child_keep_running_) {
          DoNotOptimize(busy_wait_func());
          threads_are_ready.at(i) = true;
        }
      });
    }
    // Wait until all threads have entered the loop before informing parent child is
    // ready to avoid a race.
    while (!std::all_of(threads_are_ready.begin(), threads_are_ready.end(),
                        [&](const std::atomic_bool& el) { return el == true; })) {
      usleep(100);
    }
    ASSERT_NE(-1, kill(parent_pid, kChildIsReadySignal) == -1)
        << "TearDown: kill sending kChildIsReady failed: " << strerror(errno) << ".\n";
    for (size_t i = 0; i < kNumThreads; ++i) {
      threads.at(i).join();
    }
    exit(EXIT_SUCCESS);
  }

  BoolOrTimeout StopInDesiredElfTimeout(ProcessTracer& proc, const std::string& elf_name,
                                        size_t timeout_sec = 2) {
    static BoolOrTimeout result = BoolOrTimeout::kSuccess;
    if (SIG_ERR == signal(SIGALRM, [](int) {
          result = BoolOrTimeout::kTimeout;
          // StopInDesiredElf contains signal handler for SIGINT mainly so that we could stop the
          // search easily when running unwind_for_offline and we can use it here too.
          kill(getpid(), SIGINT);
        })) {
      std::cerr << "Failed to set up signal handler for SIGALRM: " << strerror(errno) << ".\n";
      exit(EXIT_FAILURE);
    }
    alarm(timeout_sec);
    if (proc.StopInDesiredElf(elf_name)) {
      result = BoolOrTimeout::kSuccess;
    } else if (result != BoolOrTimeout::kTimeout) {
      result = BoolOrTimeout::kFail;
    }
    alarm(0);
    return result;
  }

  static constexpr size_t kNumThreads = 5;
  static constexpr int kChildIsReadySignal = SIGUSR1;
  static constexpr int kStopChildSignal = SIGUSR2;
  static inline std::atomic_bool child_is_ready_ = false;
  static inline std::atomic_bool child_keep_running_ = true;
  pid_t child_pid_;
};

static void VerifyState(pid_t tid, bool running) {
  while (true) {
    android::procinfo::ProcessInfo proc_info;
    ASSERT_TRUE(GetProcessInfo(tid, &proc_info));
    if (running) {
      if (proc_info.state == android::procinfo::kProcessStateRunning ||
          proc_info.state == android::procinfo::kProcessStateSleeping) {
        break;
      }
    } else if (proc_info.state == android::procinfo::kProcessStateStopped) {
      break;
    }
    usleep(1000);
  }
}

static void VerifyState(ProcessTracer& proc, bool running) {
  // Verify that the main thread and all threads are in the expected state.
  VerifyState(proc.pid(), running);
  if (::testing::Test::HasFatalFailure()) return;
  for (const pid_t& tid : proc.tids()) {
    VerifyState(tid, running);
    if (::testing::Test::HasFatalFailure()) return;
  }
}

TEST_P(ProcessTracerTest, stop_and_resume) {
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());

  ASSERT_TRUE(proc.Stop());
  VerifyState(proc, /*running*/ false);
  if (::testing::Test::HasFatalFailure()) return;

  ASSERT_TRUE(proc.Resume());
  VerifyState(proc, /*running*/ true);
  if (::testing::Test::HasFatalFailure()) return;
}

TEST_P(ProcessTracerTest, attach_and_detach) {
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());

  ASSERT_TRUE(proc.Attach(child_pid_));
  // Attaching to the same pid should result in failure and errno indicating that we cannot trace
  // the priocess because it is already being traced after the call to Attach().
  ASSERT_EQ(-1, ptrace(PTRACE_ATTACH, child_pid_, nullptr, nullptr));
  ASSERT_EQ(EPERM, errno);
  ASSERT_TRUE(proc.Detach(child_pid_));
  for (const pid_t& tid : proc.tids()) {
    ASSERT_TRUE(proc.Attach(tid));
    ASSERT_EQ(-1, ptrace(PTRACE_ATTACH, tid, nullptr, nullptr));
    ASSERT_EQ(EPERM, errno);
    ASSERT_TRUE(proc.Detach(tid));
  }
}

TEST_P(ProcessTracerTest, consecutive_attach_fail) {
  if (!GetParam()) GTEST_SKIP();
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());

  bool is_first_thread = true;
  for (const pid_t& tid : proc.tids()) {
    if (is_first_thread) {
      ASSERT_TRUE(proc.Attach(tid));
      is_first_thread = false;
    } else {
      ASSERT_FALSE(proc.Attach(tid));
    }
  }
}

TEST_P(ProcessTracerTest, trace_invalid_tid) {
  if (GetParam()) GTEST_SKIP();
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());
  ASSERT_FALSE(proc.Attach(getpid()));
  ASSERT_FALSE(proc.Detach(getpid()));
}

TEST_P(ProcessTracerTest, detach_with_no_attached) {
  if (GetParam()) GTEST_SKIP();
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());
  ASSERT_FALSE(proc.Detach(child_pid_));
}

TEST_P(ProcessTracerTest, uses_shared_library) {
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());

  std::string elf_name = "libunwindstack_local.so";
  ASSERT_TRUE(proc.UsesSharedLibrary(child_pid_, elf_name));
  for (const pid_t& tid : proc.tids()) {
    ASSERT_TRUE(proc.UsesSharedLibrary(tid, elf_name));
  }
}

TEST_P(ProcessTracerTest, does_not_use_shared_library) {
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());

  std::string elf_name = "libfake.so";
  ASSERT_FALSE(proc.UsesSharedLibrary(child_pid_, elf_name));
  for (const pid_t& tid : proc.tids()) {
    ASSERT_FALSE(proc.UsesSharedLibrary(tid, elf_name));
  }
}

TEST_P(ProcessTracerTest, stop_in_elf_we_use) {
  // Skip the run with is_tracing_threads=false because main thread only uses
  // the threading library.
  if (!GetParam()) GTEST_SKIP();
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());
  std::string elf_name = "libunwindstack_local.so";

  EXPECT_EQ(BoolOrTimeout::kSuccess, StopInDesiredElfTimeout(proc, elf_name));
}

TEST_P(ProcessTracerTest, timeout_when_try_to_stop_in_elf_we_do_not_use) {
  ProcessTracer proc(child_pid_, /*is_tracing_threads*/ GetParam());
  std::string elf_name = "libfake.so";

  EXPECT_EQ(BoolOrTimeout::kTimeout, StopInDesiredElfTimeout(proc, elf_name));
}

INSTANTIATE_TEST_CASE_P(IsTracingThreads, ProcessTracerTest, testing::Values(false, true));

}  // namespace
}  // namespace unwindstack
