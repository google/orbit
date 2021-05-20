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

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_TRAMPOLINE_H_