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

#include <errno.h>
#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <memory>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

#include <unwindstack/MapInfo.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Regs.h>
#include <unwindstack/Unwinder.h>

#include <android-base/file.h>
#include <android-base/stringprintf.h>
#include <procinfo/process.h>

#include "ProcessTracer.h"

namespace unwindstack {

ProcessTracer::ProcessTracer(pid_t pid, bool is_tracing_threads)
    : pid_(pid), is_tracing_threads_(is_tracing_threads) {
  if (is_tracing_threads_) is_tracing_threads_ = InitProcessTids();
}

bool ProcessTracer::InitProcessTids() {
  std::string error_msg;
  if (!android::procinfo::GetProcessTids(pid_, &tids_, &error_msg)) {
    fprintf(stderr,
            "Failed to get process tids: %s. Reverting to tracing the "
            "main thread only.\n",
            error_msg.c_str());
    return false;
  }
  if (tids_.erase(pid_) != 1) {
    fprintf(stderr,
            "Failed to erase the main thread from the thread id set. "
            "Reverting to tracing the main thread only.\n");
    return false;
  }
  return true;
}

ProcessTracer::~ProcessTracer() {
  if (cur_attached_tid_ != kNoThreadAttached) Detach(cur_attached_tid_);
  if (!is_running_) Resume();
}

bool ProcessTracer::Stop() {
  if (kill(pid_, SIGSTOP) == kKillFailed) {
    fprintf(stderr, "Failed to send stop signal to pid %d: %s\n", pid_, strerror(errno));
    return false;
  }
  usleep(1000);  // 1 ms. Without this sleep, any attempt to resume right away may fail.

  is_running_ = false;
  return true;
}

bool ProcessTracer::Resume() {
  if (kill(pid_, SIGCONT) == kKillFailed) {
    fprintf(stderr, "Failed to send continue signal to pid %d: %s\n", pid_, strerror(errno));
    return false;
  }
  usleep(1000);  // 1 ms. Without this sleep, any attempt to stop right away may fail.

  is_running_ = true;
  return true;
}

bool ProcessTracer::Detach(pid_t tid) {
  if (tid != pid_ && tids_.find(tid) == tids_.end()) {
    fprintf(stderr, "Tid %d does not belong to proc %d.\n", tid, pid_);
    return false;
  }

  if (cur_attached_tid_ == kNoThreadAttached) {
    fprintf(stderr, "Cannot detach because no thread is currently attached.\n");
    return false;
  }
  if (is_running_ && !Stop()) return false;

  if (ptrace(PTRACE_DETACH, tid, nullptr, nullptr) == kPtraceFailed) {
    fprintf(stderr, "Failed to detach from tid %d: %s\n", tid, strerror(errno));
    return false;
  }

  cur_attached_tid_ = kNoThreadAttached;
  return true;
}

bool ProcessTracer::Attach(pid_t tid) {
  if (tid != pid_ && tids_.find(tid) == tids_.end()) {
    fprintf(stderr, "Tid %d does not belong to proc %d.\n", tid, pid_);
    return false;
  }

  if (is_running_) Stop();
  if (cur_attached_tid_ != kNoThreadAttached) {
    fprintf(stderr, "Cannot attatch to tid %d. Already attached to tid %d.\n", tid,
            cur_attached_tid_);
    return false;
  }

  if (ptrace(PTRACE_ATTACH, tid, nullptr, nullptr) == kPtraceFailed) {
    fprintf(stderr, "Failed to attached to tid %d: %s\n", tid, strerror(errno));
    return false;
  }
  int status;
  if (waitpid(tid, &status, 0) == kWaitpidFailed) {
    fprintf(stderr, "Failed to stop tid %d: %s\n", tid, strerror(errno));
    return false;
  }

  cur_attached_tid_ = tid;
  return true;
}

bool ProcessTracer::StopInDesiredElf(const std::string& elf_name) {
  signal(SIGINT, [](int) { keepWaitingForPcInElf = false; });
  bool pc_in_desired_elf = true;
  do {
    if (!Attach(pid_)) return false;
    pc_in_desired_elf = ProcIsInDesiredElf(pid_, elf_name);
    if (!Detach(pid_)) return false;

    if (!pc_in_desired_elf) {
      for (pid_t tid : tids_) {
        if (!Attach(tid)) return false;
        pc_in_desired_elf = ProcIsInDesiredElf(tid, elf_name);
        if (!Detach(tid)) return false;
        if (pc_in_desired_elf) break;
      }
    }

    // If the process is not in the desired ELF, resume it for a short time, then check again.
    if (!pc_in_desired_elf) {
      Resume();
      usleep(1000);  // 1 ms
      Stop();
    }
  } while (!pc_in_desired_elf && keepWaitingForPcInElf);

  if (!pc_in_desired_elf) {
    fprintf(stderr, "\nExited while waiting for pid %d to enter %s.\n", pid_, elf_name.c_str());
    return false;
  }
  return true;
}

bool ProcessTracer::UsesSharedLibrary(pid_t pid, const std::string& desired_elf_name) {
  std::unique_ptr<Maps> maps = std::make_unique<RemoteMaps>(pid);
  if (!maps->Parse()) {
    fprintf(stderr, "Could not parse maps for pid %d.\n", pid);
    return false;
  }
  for (const auto& map : *maps) {
    if (android::base::Basename(map->name()).c_str() == desired_elf_name) return true;
  }
  return false;
}

bool ProcessTracer::ProcIsInDesiredElf(pid_t pid, const std::string& desired_elf_name) {
  std::unique_ptr<Regs> regs(Regs::RemoteGet(pid));
  if (regs == nullptr) {
    fprintf(stderr, "Unable to get remote reg data.\n");
    return false;
  }
  UnwinderFromPid unwinder(1024, pid);
  unwinder.SetRegs(regs.get());
  if (!unwinder.Init()) {
    fprintf(stderr, "Unable to intitialize unwinder.\n");
    return false;
  }
  Maps* maps = unwinder.GetMaps();
  auto map_info = maps->Find(regs->pc());
  if (map_info == nullptr) {
    regs->fallback_pc();
    map_info = maps->Find(regs->pc());
    if (map_info == nullptr) {
      return false;
    }
  }

  const std::string& current_elf_name = android::base::Basename(map_info->name()).c_str();
  bool in_desired_elf = current_elf_name == desired_elf_name;
  if (in_desired_elf) printf("pid %d is in %s! Unwinding...\n\n", pid, desired_elf_name.c_str());
  return in_desired_elf;
}
}  // namespace unwindstack
