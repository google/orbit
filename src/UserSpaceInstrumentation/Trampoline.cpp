// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Trampoline.h"

#include <absl/base/casts.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <unistd.h>

#include <cstdint>
#include <string>

#include "AllocateInTracee.h"
#include "MachineCode.h"
#include "OrbitBase/ExecuteCommand.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/UniqueResource.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

using absl::numbers_internal::safe_strtou64_base;
using orbit_base::ReadFileToString;

std::string InstructionBytesAsString(cs_insn* instruction) {
  std::string result;
  for (int i = 0; i < instruction->size; i++) {
    result = absl::StrCat(result, i == 0 ? absl::StrFormat("%#0.2x", instruction->bytes[i])
                                         : absl::StrFormat(" %0.2x", instruction->bytes[i]));
  }
  return result;
}

}  // namespace

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

ErrorMessageOr<int32_t> AddressDifferenceAsInt32(uint64_t a, uint64_t b) {
  const uint64_t abs_diff = (a > b) ? (a - b) : (b - a);
  constexpr uint64_t kAbsMaxInt32AsUint64 =
      static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
  constexpr uint64_t kAbsMinInt32AsUint64 =
      static_cast<uint64_t>(-static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
  if ((a > b && abs_diff > kAbsMaxInt32AsUint64) || (b > a && abs_diff > kAbsMinInt32AsUint64)) {
    return ErrorMessage("Difference is larger than +-2GB.");
  }
  return a - b;
}

ErrorMessageOr<RelocatedInstruction> RelocateInstruction(cs_insn* instruction, uint64_t old_address,
                                                         uint64_t new_address) {
  RelocatedInstruction result;
  if ((instruction->detail->x86.modrm & 0xC7) == 0x05) {
    // The modrm byte can encode a memory operand as a signed 32 bit displacement on the rip. See
    // "Intel 64 and IA-32 Architectures Software Developer’s Manual Vol. 2A" Chapter 2.1.
    // Specifically table 2-2.
    const int32_t old_displacement = *absl::bit_cast<int32_t*>(
        instruction->bytes + instruction->detail->x86.encoding.disp_offset);
    const uint64_t old_absolute_address = old_address + instruction->size + old_displacement;
    ErrorMessageOr<int32_t> new_displacement_or_error =
        AddressDifferenceAsInt32(old_absolute_address, new_address + instruction->size);
    if (new_displacement_or_error.has_error()) {
      return ErrorMessage(absl::StrFormat(
          "While trying to relocate an instruction with rip relative addressing the target was out "
          "of range from the trampoline. old address: %#x, new address :%#x instruction: %s",
          old_address, new_address, InstructionBytesAsString(instruction)));
    }
    result.code.resize(instruction->size);
    memcpy(result.code.data(), instruction->bytes, instruction->size);
    *absl::bit_cast<int32_t*>(result.code.data() + instruction->detail->x86.encoding.disp_offset) =
        new_displacement_or_error.value();
  } else if (instruction->detail->x86.opcode[0] == 0xeb ||
             instruction->detail->x86.opcode[0] == 0xe9) {
    // Unconditional jump to relative immediate parameter (32 bit or 8 bit).
    // We compute the absolute address and jump there:
    // jmp [rip + 0]                ff 25 00 00 00 00
    // .byte absolute_address       01 02 03 04 05 06 07 08
    const int32_t immediate =
        (instruction->detail->x86.opcode[0] == 0xe9)
            ? *absl::bit_cast<int32_t*>(instruction->bytes +
                                        instruction->detail->x86.encoding.imm_offset)
            : *absl::bit_cast<int8_t*>(instruction->bytes +
                                       instruction->detail->x86.encoding.imm_offset);
    const uint64_t absolute_address = old_address + instruction->size + immediate;
    MachineCode code;
    code.AppendBytes({0xff, 0x25}).AppendImmediate32(0x0000000).AppendImmediate64(absolute_address);
    result.code = code.GetResultAsVector();
    result.position_of_absolute_address = 6;
  } else if (instruction->detail->x86.opcode[0] == 0xe8) {
    // Call function at relative immediate parameter.
    // We compute the absolute address of the called function and call it like this:
    // Call [rip+2]                 ff 15 02 00 00 00
    // jmp label;                   eb 08
    // .byte absolute_address       01 02 03 04 05 06 07 08
    // label:
    const int32_t immediate = *absl::bit_cast<int32_t*>(
        instruction->bytes + instruction->detail->x86.encoding.imm_offset);
    const uint64_t absolute_address = old_address + instruction->size + immediate;
    MachineCode code;
    code.AppendBytes({0xff, 0x15})
        .AppendImmediate32(0x00000002)
        .AppendBytes({0xeb, 0x08})
        .AppendImmediate64(absolute_address);
    result.code = code.GetResultAsVector();
    result.position_of_absolute_address = 8;
  } else if ((instruction->detail->x86.opcode[0] & 0xf0) == 0x70) {
    // 0x7? are conditional jumps to an 8 bit immediate.
    // We invert the condition of the jump and construct the following code sequence.
    // j(!cc) 0x0e                  7? 0e
    // jmp [rip + 0]                ff 25 00 00 00 00
    // .byte absolute_address       01 02 03 04 05 06 07 08
    const int8_t immediate =
        *absl::bit_cast<int8_t*>(instruction->bytes + instruction->detail->x86.encoding.imm_offset);
    const uint64_t absolute_address = old_address + instruction->size + immediate;
    MachineCode code;
    // Inverting the last bit negates the condidion for the jump (e.g. 0x74 is "jump if equal", 0x75
    // is "jump if not equal").
    const uint8_t opcode = 0x01 ^ instruction->detail->x86.opcode[0];
    code.AppendBytes({opcode, 0x0e})
        .AppendBytes({0xff, 0x25, 0x00, 0x00, 0x00, 0x00})
        .AppendImmediate64(absolute_address);
    result.code = code.GetResultAsVector();
    result.position_of_absolute_address = 8;
  } else if (instruction->detail->x86.opcode[0] == 0x0f &&
             (instruction->detail->x86.opcode[1] & 0xf0) == 0x80) {
    // 0x0f 0x8? are conditional jumps to a 32 bit immediate
    // We invert the condition of the jump and construct the following code sequence.
    // j(!cc) 0x0e                  7? 0e
    // jmp [rip + 0]                ff 25 00 00 00 00
    // .byte absolute_address       01 02 03 04 05 06 07 08
    const int32_t immediate = *absl::bit_cast<int32_t*>(
        instruction->bytes + instruction->detail->x86.encoding.imm_offset);
    const uint64_t absolute_address = old_address + instruction->size + immediate;
    MachineCode code;
    // Inverting the last bit negates the condidion for the jump. We need a jump to an eight bit
    // immediate (opcode 0x7?).
    const uint8_t opcode = 0x70 | (0x01 ^ (instruction->detail->x86.opcode[1] & 0x0f));
    code.AppendBytes({opcode, 0x0e})
        .AppendBytes({0xff, 0x25, 0x00, 0x00, 0x00, 0x00})
        .AppendImmediate64(absolute_address);
    result.code = code.GetResultAsVector();
    result.position_of_absolute_address = 8;
  } else if ((instruction->detail->x86.opcode[0] & 0xfc) == 0xe0) {
    // 0xe{0, 1, 2, 3} loops to an 8 bit immediate. These instructions are not used by modern
    // compilers. Depending on whether we ever see them we might implement something eventually.
    return ErrorMessage(
        absl::StrFormat("Relocating a loop instruction is not supported. Instruction: %s",
                        InstructionBytesAsString(instruction)));
  } else {
    // All other instructions can just be copied.
    result.code.resize(instruction->size);
    memcpy(result.code.data(), instruction->bytes, instruction->size);
  }

  return result;
}

}  // namespace orbit_user_space_instrumentation
