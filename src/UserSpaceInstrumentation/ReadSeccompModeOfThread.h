// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_READ_SECCOMP_MODE_OF_THREAD_H_
#define USER_SPACE_INSTRUMENTATION_READ_SECCOMP_MODE_OF_THREAD_H_

#include <unistd.h>

#include <optional>

namespace orbit_user_space_instrumentation {

// Retrieves the seccomp mode of a thread.
[[nodiscard]] std::optional<int> ReadSeccompModeOfThread(pid_t tid);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_READ_SECCOMP_MODE_OF_THREAD_H_
