// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ANY_THREAD_IS_IN_STRICT_SECCOMP_MODE_H_
#define USER_SPACE_INSTRUMENTATION_ANY_THREAD_IS_IN_STRICT_SECCOMP_MODE_H_

#include <absl/container/flat_hash_set.h>
#include <sys/types.h>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Returns true if and only if at least one thread of the threads of the specified process in
// strict seccomp mode, false otherwise. While not required, it generally assumes that the process
// has been attached to and stopped with ptrace, such that no threads could spawn in the meantime.
[[nodiscard]] bool AnyThreadIsInStrictSeccompMode(pid_t pid);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ANY_THREAD_IS_IN_STRICT_SECCOMP_MODE_H_