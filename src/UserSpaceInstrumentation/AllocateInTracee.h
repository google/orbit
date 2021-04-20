// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEE_H_
#define USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEE_H_

#include <sys/types.h>

#include <cstdint>
#include <functional>

#include "OrbitBase/Result.h"
#include "OrbitBase/UniqueResource.h"

namespace orbit_user_space_instrumentation {

// Allocate `size` bytes of memory in the tracee's address space using mmap. The memory will have
// read, write, and execute permissions. The memory allocated will start at `address`. `address`
// needs to be aligned to page boundaries. If the memory mapping can not be placed at `address` an
// error is returned.
// If `address` is zero the placement of memory will be arbitray (compare the documenation of mmap:
// https://man7.org/linux/man-pages/man2/mmap.2.html). Assumes we are already attached to the tracee
// `pid` e.g. using `AttachAndStopProcess`.
[[nodiscard]] ErrorMessageOr<uint64_t> AllocateInTracee(pid_t pid, uint64_t address, uint64_t size);

// Free address range previously allocated with AllocateInTracee using munmap.
// Assumes we are already attached to the tracee `pid` e.g. using `AttachAndStopProcess`.
[[nodiscard]] ErrorMessageOr<void> FreeInTracee(pid_t pid, uint64_t address, uint64_t size);

// Same as `AllocateInTracee` above but wraps the memory in a unique_resource that handles
// deallocating the memory. Note that we still need to be attached (or attached again) to the tracee
// when the unique_resource goes out of scope.
[[nodiscard]] ErrorMessageOr<orbit_base::unique_resource<uint64_t, std::function<void(uint64_t)>>>
AllocateInTraceeAsUniqueResource(pid_t pid, uint64_t address, uint64_t size);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEE_H_