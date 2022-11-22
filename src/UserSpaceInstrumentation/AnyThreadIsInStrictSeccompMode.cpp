// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserSpaceInstrumentation/AnyThreadIsInStrictSeccompMode.h"

#include <linux/seccomp.h>

#include <optional>
#include <vector>

#include "OrbitBase/GetProcessIds.h"
#include "ReadSeccompModeOfThread.h"

namespace orbit_user_space_instrumentation {

// With user space instrumentation and manual instrumentation, if the target process has at least
// one thread in strict seccomp mode, we can have two different problems:
// - If the main thread is in strict mode, the injection is certain to get it killed as it uses
//   multiple system calls; we could choose a different thread for the injection, but...
// - If any thread that is in strict mode hits the instrumentation, the instrumentation functions
//   (`EntryPayload`/`ExitPayload`, Orbit API's functions) are also likely to get the thread killed.
// We use this function in order to detect whether any thread is in strict seccomp mode at the
// moment of injection. If that's the case, we simply refrain from proceeding with the injection. Of
// course the target could spawn a thread that switches to strict mode after the start of the
// capture, but this is the best we can do.
bool AnyThreadIsInStrictSeccompMode(pid_t pid) {
  for (pid_t tid : orbit_base::GetTidsOfProcess(pid)) {
    const std::optional<int> seccomp_mode = ReadSeccompModeOfThread(tid);
    if (seccomp_mode.has_value() && seccomp_mode.value() == SECCOMP_MODE_STRICT) {
      return true;
    }
  }
  return false;
}

}  // namespace orbit_user_space_instrumentation
