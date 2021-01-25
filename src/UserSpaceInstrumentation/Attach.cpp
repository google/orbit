// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/Attach.h"

#include <errno.h>
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
#include "absl/strings/str_format.h"

namespace orbit_user_space_instrumentation {

using orbit_base::GetTidsOfProcess;

namespace {

bool ThreadExistsInProcess(pid_t pid, pid_t tid) {
  const auto process_tids = orbit_base::GetTidsOfProcess(pid);
  return std::find(process_tids.begin(), process_tids.end(), tid) != process_tids.end();
}

// Attaches to the thread using ptrace.
// Returns true if the thread was halted and false if the thread did not exist anymore.
[[nodiscard]] ErrorMessageOr<bool> AttachAndStopThread(pid_t pid, pid_t tid) {
  if (ptrace(PTRACE_ATTACH, tid, 0, 0) < 0) {
    if (ThreadExistsInProcess(pid, tid)) {
      return ErrorMessage(
          absl::StrFormat("Can not attach to tid \"%d\": \"%s\"", tid, strerror(errno)));
    } else {
      return false;
    }
  }
  // Wait for the traced thread to stop. Timeout after one second.
  int timeout = 1000;
  while (true) {
    const int result_waitpid = waitpid(tid, nullptr, WNOHANG);
    if (result_waitpid == -1) {
      if (ThreadExistsInProcess(pid, tid)) {
        return ErrorMessage(
            absl::StrFormat("Can not attach to tid \"%d\": \"%s\"", tid, strerror(errno)));
      } else {
        return false;
      }
    }
    if (result_waitpid > 0) {
      // waitpid returns greater zero occationally for terminated threads. We catch this here so we
      // don't count them as succefully halted.
      // This is not understood. The check here is a workaround. Note that this case is extremely
      // rare in practice - the thread needs to finish exactly when we attach to the process.
      return ThreadExistsInProcess(pid, tid);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    if (--timeout == 0) {
      ptrace(PTRACE_DETACH, tid, nullptr, 0);
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
      OUTCOME_TRY(result, AttachAndStopThread(pid, tid));
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
    ptrace(PTRACE_DETACH, tid, nullptr, 0);
  }
}

}  // namespace orbit_user_space_instrumentation