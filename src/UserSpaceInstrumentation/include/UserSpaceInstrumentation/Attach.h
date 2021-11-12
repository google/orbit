// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ATTACH_H_
#define USER_SPACE_INSTRUMENTATION_ATTACH_H_

#include <absl/container/flat_hash_set.h>
#include <sys/types.h>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Attaches to and stops all threads of the process `pid` using `PTRACE_ATTACH`. Being attached to a
// process using this function is a precondition for using any of the tooling provided here.
// Returns the set of thread ids of the threads that were stopped.
[[nodiscard]] ErrorMessageOr<absl::flat_hash_set<pid_t>> AttachAndStopProcess(pid_t pid);

// Attaches to and stops all threads of the process `pid` that are not already in
// `already_halted_tids`. It can be used to stop threads that spawned while already attached to a
// process.
// Returns the new set of thread ids of all halted threads (old and new).
[[nodiscard]] ErrorMessageOr<absl::flat_hash_set<pid_t>> AttachAndStopNewThreadsOfProcess(
    pid_t pid, absl::flat_hash_set<pid_t> already_halted_tids);

// Detaches from all threads of the process `pid` and continues the execution.
[[nodiscard]] ErrorMessageOr<void> DetachAndContinueProcess(pid_t pid);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ATTACH_H_