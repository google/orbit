// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Trampoline.h"

#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "AllocateInTracee.h"
#include "OrbitBase/ExecuteCommand.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

using absl::numbers_internal::safe_strtou64_base;
using orbit_base::ReadFileToString;

bool DoAddressRangesOverlap(const AddressRange& a, const AddressRange& b) {
  return !(b.end <= a.start || b.start >= a.end);
}

std::optional<size_t> LowestIntersectingAddressRange(const std::vector<AddressRange>& ranges_sorted,
                                                     const AddressRange& range) {
  for (size_t i = 0; i < ranges_sorted.size(); i++) {
    if (DoAddressRangesOverlap(ranges_sorted[i], range)) {
      return i;
    }
  }
  return std::nullopt;
}

std::optional<size_t> HighestIntersectingAddressRange(
    const std::vector<AddressRange>& ranges_sorted, const AddressRange& range) {
  for (int i = ranges_sorted.size() - 1; i >= 0; i--) {
    if (DoAddressRangesOverlap(ranges_sorted[i], range)) {
      return i;
    }
  }
  return std::nullopt;
}

ErrorMessageOr<std::vector<AddressRange>> GetUnavailableAddressRanges(pid_t pid) {
  std::vector<AddressRange> result;
  OUTCOME_TRY(mmap_min_addr, ReadFileToString("/proc/sys/vm/mmap_min_addr"));
  uint64_t mmap_min_addr_as_uint64 = 0;
  if (!absl::SimpleAtoi(mmap_min_addr, &mmap_min_addr_as_uint64)) {
    return ErrorMessage("Failed to parse /proc/sys/vm/mmap_min_addr");
  }
  result.emplace_back(0, mmap_min_addr_as_uint64);

  OUTCOME_TRY(maps, ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));
  std::vector<std::string> lines = absl::StrSplit(maps, '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 1) {
      continue;
    }
    std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) {
      continue;
    }
    uint64_t address_begin = 0;
    uint64_t address_end = 0;
    if (!safe_strtou64_base(addresses[0], &address_begin, 16) ||
        !safe_strtou64_base(addresses[1], &address_end, 16)) {
      continue;
    }
    CHECK(address_begin < address_end);
    // Join with previous segment ...
    if (result.back().end == address_begin) {
      result.back().end = address_end;
      continue;
    }
    // ... or append as new segment.
    result.emplace_back(address_begin, address_end);
  }
  return result;
}

ErrorMessageOr<AddressRange> FindAddressRangeForTrampoline(
    const std::vector<AddressRange>& unavailable_ranges, const AddressRange& code_range,
    uint64_t size) {
  constexpr uint64_t kMax32BitOffset = INT32_MAX;
  constexpr uint64_t kMax64BitAdress = UINT64_MAX;
  const uint64_t page_size = sysconf(_SC_PAGE_SIZE);

  FAIL_IF(unavailable_ranges.size() == 0 || unavailable_ranges[0].start != 0,
          "First entry at unavailable_ranges needs to start at zero. Use result of "
          "GetUnavailableAddressRanges.");

  // Try to fit an interval of length `size` below `code_range`.
  auto optional_range_index = LowestIntersectingAddressRange(unavailable_ranges, code_range);
  if (!optional_range_index) {
    return ErrorMessage(absl::StrFormat("code_range %#x-%#x is not in unavailable_ranges.",
                                        code_range.start, code_range.end));
  }
  while (optional_range_index.value() > 0) {
    // Place directly to the left of the take interval we are in...
    uint64_t trampoline_address = unavailable_ranges[optional_range_index.value()].start - size;
    // ... but round down to page boundary.
    trampoline_address = (trampoline_address / page_size) * page_size;
    AddressRange trampoline_range = {trampoline_address, trampoline_address + size};
    optional_range_index = LowestIntersectingAddressRange(unavailable_ranges, trampoline_range);
    if (!optional_range_index) {
      // We do not intersect any taken interval. Check if we are close enough to code_range:
      // code_range is above trampoline_range we will need to jmp back and forth in these ranges
      // with 32 bit offsets. If no distance is greater than 0x7fffffff this is safe.
      if (code_range.end - trampoline_range.start <= kMax32BitOffset) {
        return trampoline_range;
      }
      // If we are already beyond the close range there is no need to go any further.
      break;
    }
  }

  // Try to fit an interval of length `size` above `code_range`.
  optional_range_index = HighestIntersectingAddressRange(unavailable_ranges, code_range);
  if (!optional_range_index) {
    return ErrorMessage(absl::StrFormat("code_range %#x-%#x is not in unavailable_ranges.",
                                        code_range.start, code_range.end));
  }
  do {
    const uint64_t trampoline_address =
        ((unavailable_ranges[optional_range_index.value()].end + (page_size - 1)) / page_size) *
        page_size;
    // Check if we ran out of address space.
    if (trampoline_address >= kMax64BitAdress - size) {
      break;
    }
    AddressRange trampoline_range = {trampoline_address, trampoline_address + size};
    optional_range_index = HighestIntersectingAddressRange(unavailable_ranges, trampoline_range);
    if (!optional_range_index) {
      // We do not intersect any taken interval. Check if we are close enough to code_range:
      // code_range is below trampoline_range we will need to jump back and forth in these ranges
      // with 32 bit offsets. If no distance is greater than 0x7fffffff this is safe.
      if (trampoline_range.end - code_range.start <= kMax32BitOffset) {
        return trampoline_range;
      }
      // If we are already beyond the close range there is no need to go any further.
      break;
    }
  } while (true);
  return ErrorMessage(absl::StrFormat("No place to fit %u bytes close to code range %#x-%#x.", size,
                                      code_range.start, code_range.end));
}

ErrorMessageOr<uint64_t> AllocateMemoryForTrampolines(pid_t pid, const AddressRange& code_range,
                                                      uint64_t size) {
  OUTCOME_TRY(unavailable_ranges, GetUnavailableAddressRanges(pid));
  OUTCOME_TRY(address_range, FindAddressRangeForTrampoline(unavailable_ranges, code_range, size));
  return AllocateInTracee(pid, address_range.start, size);
}

ErrorMessageOr<absl::flat_hash_set<uint64_t>> AllInstructionPointersFromProcess(pid_t pid) {
  absl::flat_hash_set<uint64_t> result;
  std::vector<pid_t> tids = orbit_base::GetTidsOfProcess(pid);
  for (pid_t tid : tids) {
    RegisterState registers;
    OUTCOME_TRY(registers.BackupRegisters(tid));
    result.insert(registers.GetGeneralPurposeRegisters()->x86_64.rip);
  }
  return result;
}

std::optional<int> LengthOfOverriddenInstructions(csh handle, const std::vector<uint8_t>& code,
                                                  int bytes_to_override) {
  cs_insn* instruction = cs_malloc(handle);
  CHECK(instruction != nullptr);

  const uint8_t* code_pointer = code.data();
  size_t code_size = code.size();
  uint64_t address = 0;
  int length = 0;

  while (cs_disasm_iter(handle, &code_pointer, &code_size, &address, instruction)) {
    length += instruction->size;
    if (length >= bytes_to_override) break;
  }
  cs_free(instruction, 1);

  // Function too short?
  if (length < bytes_to_override) {
    return {};
  }
  return length;
}

}  // namespace orbit_user_space_instrumentation
