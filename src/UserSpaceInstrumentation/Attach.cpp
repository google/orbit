// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/Attach.h"

#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>
#include <errno.h>
#include <stdint.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <chrono>
#include <memory>
#include <set>

#include <string>
#include <thread>
#include <vector>

#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"

namespace orbit_user_space_instrumentation {

using orbit_base::GetTidsOfProcess;

namespace {

// Attaches to the thread using ptrace.
// Returns true if the thread was halted and false if the thread did not exist anymore.
[[nodiscard]] ErrorMessageOr<bool> AttachAndStopThread(pid_t tid) {
  if (ptrace(PTRACE_ATTACH, tid, nullptr, nullptr) == -1) {
    // If tid has ended already we get ESRCH; if the thread was in 'exit state' we get EPERM.
    // There are a bunch of other (non-relevant) cases. I haven't found documentation on this but it
    // can be looked up in the function 'ptrace_attach' in 'ptrace.c' in the kernel sources.
    if (errno == ESRCH || errno == EPERM) {
      return false;
    }
    return ErrorMessage(
        absl::StrFormat("PTRACE_ATTACH failed for %d: \"%s\"", tid, SafeStrerror(errno)));
  }
  // Wait for the traced thread to stop. Timeout after one second.
  constexpr int kMaxAttempts = 1000;
  for (int i = 0; i < kMaxAttempts; i++) {
    int stat_val = 0;
    const int waitpid_result = waitpid(tid, &stat_val, WNOHANG);
    if (waitpid_result == -1) {
      return ErrorMessage(absl::StrFormat("Wait for thread to get traced failed for tid %d: \"%s\"",
                                          tid, SafeStrerror(errno)));
    }
    if (waitpid_result > 0) {
      // Occasionally the thread is active during PTRACE_ATTACH but terminates before it gets
      // descheduled. So waitpid returns on exit of the thread instead of the expected stop.
      if (WIFEXITED(stat_val)) {
        return false;
      }
      if (WIFSTOPPED(stat_val)) {
        return true;
      }
      return ErrorMessage(absl::StrFormat(
          "Wait for thread to get traced yielded unexpected result for tid %d: %d", tid, stat_val));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  ptrace(PTRACE_DETACH, tid, nullptr, nullptr);
  return ErrorMessage(absl::StrFormat("Waiting for the traced thread %d to stop timed out.", tid));
}

}  // namespace

[[nodiscard]] ErrorMessageOr<void> AttachAndStopProcess(pid_t pid) {
  auto process_tids = GetTidsOfProcess(pid);
  absl::flat_hash_set<pid_t> halted_tids;
  // Note that the process is still running - it can spawn and end threads at this point.
  while (process_tids.size() != halted_tids.size()) {
    for (const auto tid : process_tids) {
      if (halted_tids.contains(tid)) {
        continue;
      }
      OUTCOME_TRY(result, AttachAndStopThread(tid));
      if (result) {
        halted_tids.insert(tid);
      }
    }
    process_tids = GetTidsOfProcess(pid);
  }
  return outcome::success();
}

[[nodiscard]] ErrorMessageOr<void> DetachAndContinueProcess(pid_t pid) {
  auto tids = GetTidsOfProcess(pid);
  for (auto tid : tids) {
    if (ptrace(PTRACE_DETACH, tid, nullptr, nullptr) == -1) {
      return ErrorMessage(absl::StrFormat("Error while detaching from thread %d: \"%s\"", tid,
                                          SafeStrerror(errno)));
    }
  }
  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation