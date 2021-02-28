// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEe_H_
#define USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEe_H_

#include <sys/types.h>

#include <cstdint>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Allocate `size` bytes of memory in the tracee's address space using mmap. The memory will have
// read, write, and execute permissions.
// Assumes we are already attached to the tracee `pid` e.g. using `AttachAndStopProcess`.
[[nodiscard]] ErrorMessageOr<uint64_t> AllocateInTracee(pid_t pid, uint64_t size);

// Free address range previously allocated with AllocateInTracee using munmap.
// Assumes we are already attached to the tracee `pid` e.g. using `AttachAndStopProcess`.
[[nodiscard]] ErrorMessageOr<void> FreeInTracee(pid_t pid, uint64_t address, uint64_t size);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEe_H_