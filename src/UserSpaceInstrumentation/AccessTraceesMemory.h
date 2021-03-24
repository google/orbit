// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ACCESS_TRACEES_MEMORY_H_
#define USER_SPACE_INSTRUMENTATION_ACCESS_TRACEES_MEMORY_H_

#include <sys/types.h>

#include <cstdint>
#include <vector>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Read `length` bytes from process `pid` starting at `address_start`. `length` will be rounded up
// to a multiple of eight.
// Assumes we are already attached to the tracee `pid` e.g. using `AttachAndStopProcess`.
[[nodiscard]] ErrorMessageOr<std::vector<uint8_t>> ReadTraceesMemory(pid_t pid,
                                                                     uint64_t address_start,
                                                                     uint64_t length);

// Write `bytes` into memory of process `pid` starting from `address_start`.
// Note that we write multiples of eight bytes at once. If length of `bytes` is not a multiple of
// eight the remaining bytes are zeroed out.
// Assumes we are already attached to the tracee `pid` e.g. using `AttachAndStopProcess`.
[[nodiscard]] ErrorMessageOr<void> WriteTraceesMemory(pid_t pid, uint64_t address_start,
                                                      const std::vector<uint8_t>& bytes);

// Returns the address range of the first executable memory region. In every case I encountered this
// was the second line in the `maps` file corresponding to the code of the process we look at.
// However we don't really care. So keeping it general and just searching for an executable region
// is probably helping stability.
// Optionally one can specify `exclude_address`. This prevents the method from returning an address
// range containing `exclude_address`.
using AddressRange = std::pair<uint64_t, uint64_t>;
[[nodiscard]] ErrorMessageOr<AddressRange> GetFirstExecutableMemoryRegion(
    pid_t pid, uint64_t exclude_address = 0);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ACCESS_TRACEES_MEMORY_H_