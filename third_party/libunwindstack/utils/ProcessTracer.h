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

#pragma once

#include <sys/types.h>

#include <set>
#include <string>
#include <unordered_set>

namespace unwindstack {

// ProcessTracer objects abstract operations for tracing a process and its threads with ptrace(2).
class ProcessTracer final {
 public:
  ProcessTracer(pid_t pid, bool is_tracing_threads);

  ~ProcessTracer();

  // ProcessTracer instances are moveable but not copyable because they manage the
  // state of a process.
  ProcessTracer(const ProcessTracer&) = delete;
  ProcessTracer& operator=(const ProcessTracer&) = delete;
  ProcessTracer(ProcessTracer&&) = default;
  ProcessTracer& operator=(ProcessTracer&&) = default;

  pid_t pid() const { return pid_; }

  const std::set<pid_t>& tids() const { return tids_; }

  bool IsTracingThreads() const { return is_tracing_threads_; }

  bool Stop();

  bool Resume();

  // Like ptrace, it is required to call ProcessTracer::Detach before calling ProcessTracer::Attach
  // on a different thread of the same process.
  bool Detach(pid_t tid);

  bool Attach(pid_t tid);

  // This method for determining whether a thread is currently executing instructions from a
  // desired ELF is not the most time efficient solution. In the interest of simplicity and
  // limiting memory usage, the UnwinderFromPid, Regs, and Maps instances constructed for
  // in each check (loop iteration) are thrown away.
  //
  // A SIGINT signal handler is set up to allow the user to gracefully exit with CTRL-C if they
  // decide that they no longer want to wait for the process to enter the desired ELF.
  bool StopInDesiredElf(const std::string& elf_name);

  // `desired_elf_name` should match the filename of the path (the component following the final
  // '/') corresponding to the shared library as indicated in /proc/pid/maps.
  static bool UsesSharedLibrary(pid_t pid, const std::string& desired_elf_name);

 private:
  static bool ProcIsInDesiredElf(pid_t tid, const std::string& desired_elf_name);

  // Initialize tids_ such that the main thread is the first element and
  // the remaining tids are in order from least to greatest.
  bool InitProcessTids();

  static constexpr pid_t kNoThreadAttached = -2;
  static constexpr pid_t kKillFailed = -1;
  static constexpr pid_t kPtraceFailed = -1;
  static constexpr pid_t kWaitpidFailed = -1;
  static inline std::atomic_bool keepWaitingForPcInElf = true;
  const pid_t pid_;
  bool is_tracing_threads_ = false;
  std::set<pid_t> tids_;
  bool is_running_ = true;
  pid_t cur_attached_tid_ = kNoThreadAttached;
};
}  // namespace unwindstack
