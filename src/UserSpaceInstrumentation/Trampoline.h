// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_TRAMPOLINE_H_
#define USER_SPACE_INSTRUMENTATION_TRAMPOLINE_H_

#include <absl/container/flat_hash_set.h>
#include <capstone/capstone.h>
#include <sys/types.h>

#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

#include "AddressRange.h"
#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Returns true if the ranges overlap (touching ranges do not count as overlapping). Assumes that
// the ranges are well formed (start < end).
[[nodiscard]] bool DoAddressRangesOverlap(const AddressRange& a, const AddressRange& b);

// Returns the index of the lowest range in `ranges_sorted` that is intersecting with `range`.
// `ranges_sorted` needs to contain non-overlapping ranges in ascending order (as provided by
// `GetUnavailableAddressRanges`).
[[nodiscard]] std::optional<size_t> LowestIntersectingAddressRange(
    const std::vector<AddressRange>& ranges_sorted, const AddressRange& range);

// Returns the index of the highest range in `ranges_sorted` that is intersecting with `range`.
// `ranges_sorted` needs to contain non-overlapping ranges in ascending order (as provided by
// `GetUnavailableAddressRanges`).
[[nodiscard]] std::optional<size_t> HighestIntersectingAddressRange(
    const std::vector<AddressRange>& ranges_sorted, const AddressRange& range);

// Parses the /proc/pid/maps file of a process and returns all the taken address ranges (joining
// directly neighboring ones). We also add a range [0, /proc/sys/vm/mmap_min_addr] to block the
// lowest addresses in the process space which mmap cannot use.
[[nodiscard]] ErrorMessageOr<std::vector<AddressRange>> GetUnavailableAddressRanges(pid_t pid);

// Finds an empty address range not overlapping with anything in `unavailable_ranges` of a given
// `size` suitable to allocate the trampolines close to `code_range`. "Close to" in this context
// means that the trampolines can't be more than a 32 bit offset away from the `code_range` (+-2GB)
// such that we can jump back and forth from the trampolines to the code using relative 32 bit
// addresses.
// `unavailable_ranges` needs to contain non-overlapping ranges in ascending order; the smallest
// range needs to start at zero (as provided by `GetUnavailableAddressRanges`).
[[nodiscard]] ErrorMessageOr<AddressRange> FindAddressRangeForTrampoline(
    const std::vector<AddressRange>& unavailable_ranges, const AddressRange& code_range,
    uint64_t size);

// Allocates `size` bytes in the tracee close to `code_range`. The memory segment will be placed
// such that we can jump from any position in the memory segment to any position in `code_range`
// (and vice versa) by relative jumps using 32 bit offsets.
[[nodiscard]] ErrorMessageOr<uint64_t> AllocateMemoryForTrampolines(pid_t pid,
                                                                    const AddressRange& code_range,
                                                                    uint64_t size);

// Returns the set of the instruction pointers from all the threads of a halted tracee.
[[nodiscard]] ErrorMessageOr<absl::flat_hash_set<uint64_t>> GetInstructionPointersFromProcess(
    pid_t pid);

// When overwriting instructions at the beginning of a function we need to know the number of bytes
// taken up by the instructions we hit with the write operation. Example:
//
// Beginning of the code we want to overwrite:

// 0x5583ba2837a0: push         rbp
// 0x5583ba2837a1: mov          rbp, rsp
// 0x5583ba2837a4: sub          rsp, 0x10
// 0x5583ba2837a8: mov          rax, qword ptr fs:[0x28]
//

// We'd like to overwrite from 0x5583ba2837a0 with a jmp which is five bytes long:
// 0x5583ba2837a0: jmp          0x5583ba0bf000
// 0x5583ba2837a5: (last three bytes of the encoding of `sub rsp, 0x10`)
//
// Since the instruction lengths do not align we end up with three bytes of garbage, the next intact
// instruction starts at 0x5583ba2837a8. So LengthOfOverwrittenInstructions would return eight.
//
// `handle` is a handle to the capstone library returned by cs_open. `code` contains the machine
// code of the function that should be overwritten. `bytes_to_overwrite` is the number of bytes to
// overwrite at the beginning of `code`.
// The return value is either the number of bytes as described above or nullopt if there was a
// problem with the function. That might either be that the function is too short or that we were
// unable to disassemble enough code to cover `bytes_to_overwrite` bytes.
[[nodiscard]] std::optional<int> LengthOfOverwrittenInstructions(csh handle,
                                                                 const std::vector<uint8_t>& code,
                                                                 int bytes_to_overwrite);

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_TRAMPOLINE_H_