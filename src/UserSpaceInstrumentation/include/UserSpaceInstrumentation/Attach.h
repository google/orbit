// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ATTACH_H_
#define USER_SPACE_INSTRUMENTATION_ATTACH_H_

#include <sys/types.h>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Attaches to process `pid` using ptrace.
// Attaches to and stopps all threads of the process using `PTRACE_ATTACH`. Being attached to a
// process using this function is a precondition for using any of the tooling provided here.
[[nodiscard]] ErrorMessageOr<void> AttachAndStopProcess(pid_t pid);

// Detach from the process `pid.`
// Detaches from all threads of the process and continues the execution.
[[nodiscard]] ErrorMessageOr<void> DetachAndContinueProcess(pid_t pid);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ATTACH_H_