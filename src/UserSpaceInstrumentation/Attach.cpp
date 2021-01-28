// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/Attach.h"

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
    } else {
      return ErrorMessage(
          absl::StrFormat("PTRACE_ATTACH failed for \"%d\": \"%s\"", tid, strerror(errno)));
    }
  }
  // Wait for the traced thread to stop. Timeout after one second.
  int32_t timeout_ms = 1000;
  while (true) {
    int stat_val = 0;
    const int result_waitpid = waitpid(tid, &stat_val, WNOHANG);
    if (result_waitpid == -1) {
      return ErrorMessage(absl::StrFormat(
          "Wait for thread to get traced failed for tid \"%d\": \"%s\"", tid, strerror(errno)));
    }
    if (result_waitpid > 0) {
      // Occasionally the thread is active during PTRACE_ATTACH but terminates before it gets
      // descheduled. So waitpid returns on exit of the thread instead of the expected stop.
      if (WIFEXITED(stat_val)) {
        return false;
      } else if (WIFSTOPPED(stat_val)) {
        return true;
      } else {
        return ErrorMessage(absl::StrFormat(
            "Wait for thread to get traced yielded unexpected result for tid \"%d\": \"%d\"", tid,
            stat_val));
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (--timeout_ms == 0) {
      ptrace(PTRACE_DETACH, tid, nullptr, nullptr);
      return ErrorMessage(
          absl::StrFormat("Waiting for the traced thread \"%d\" to stop timed out.", tid));
    }
  }
  return true;
}

}  // namespace

[[nodiscard]] ErrorMessageOr<void> AttachAndStopProcess(pid_t pid) {
  auto process_tids = GetTidsOfProcess(pid);
  std::set<pid_t> halted_tids;
  // Note that the process is still running - it can spawn and end threads at this point.
  while (true) {
    for (const auto tid : process_tids) {
      if (halted_tids.count(tid) == 1) {
        continue;
      }
      OUTCOME_TRY(result, AttachAndStopThread(tid));
      if (result) {
        halted_tids.insert(tid);
      }
    }
    // Update the tids; if there are new ones run another round and stop them.
    process_tids = GetTidsOfProcess(pid);
    if (process_tids.size() == halted_tids.size()) {
      break;
    }
  }
  return outcome::success();
}

void DetachAndContinueProcess(pid_t pid) {
  auto tids = GetTidsOfProcess(pid);
  for (auto tid : tids) {
    ptrace(PTRACE_DETACH, tid, nullptr, nullptr);
  }
}

}  // namespace orbit_user_space_instrumentation