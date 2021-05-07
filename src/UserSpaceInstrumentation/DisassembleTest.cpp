// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <capstone/capstone.h>
#include <cpuid.h>
#include <dlfcn.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <random>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AddressRange.h"
#include "AllocateInTracee.h"
#include "ElfUtils/ElfFile.h"
#include "ElfUtils/LinuxMap.h"
#include "MachineCode.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/TestUtils.h"
#include "RegisterState.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"
#include "UserSpaceInstrumentation/InjectLibraryInTracee.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::HasError;
using orbit_base::HasValue;
using orbit_elf_utils::ElfFile;
using orbit_elf_utils::ReadModules;
using testing::ElementsAreArray;

// Number of bytes to overwrite at the bginning of the function. Relative jump to a signed 32 bit
// offset looks like this:
// jmp 01020304         e9 04 03 02 01
size_t kSizeOfJmp = 5;

// We relocate most `kSizeOfJmp` instructions. When relocating we are not adding any
// instructions so there are at most `kSizeOfJmp` relocated instructions in the trampoline.
// The longest possible instruction in x64 is 16 bytes. So we get this (very generous) upper bound.
size_t kMaxRelocatedPrologSize = kSizeOfJmp * 16;

// The function backup should contain all the instructions that can be hit by the overwritten
// `kSizeOfJmp` bytes. In the worst case the last byte is the beginning of an instruction of
// maximum length (which is 16).
size_t kMaxFunctionPrologBackupSize = kSizeOfJmp - 1 + 16;

uint64_t g_a = 0;

extern "C" __attribute__((noinline)) int DoSomething(int i) {
  g_a++;
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dis(1, 6);
  std::vector<int> v(11);
  std::generate(v.begin(), v.end(), [&]() { return dis(gen); });
  int sum = std::accumulate(v.begin(), v.end(), 0);
  __asm__ __volatile__(
      ".byte 0xeb \n\t"
      ".byte 0x01 \n\t"
      "nop\n\t"
      "nop\n\t"
      :
      :
      :);

  return i + sum;
}

std::string InstructionBytesAsString(cs_insn* instruction) {
  std::string result;
  for (int i = 0; i < instruction->size; i++) {
    result = absl::StrCat(result, i == 0 ? absl::StrFormat("%#0.2x", instruction->bytes[i])
                                         : absl::StrFormat(" %0.2x", instruction->bytes[i]));
  }
  return result;
}

[[nodiscard]] bool HasAvx() {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  return __get_cpuid(0x01, &eax, &ebx, &ecx, &edx) && (ecx & bit_AVX);
}

}  // namespace

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

void AppendBackupCode(MachineCode& trampoline) {
  // This code is executed immediatly after the control is passed to the instrumented function. The
  // top of the stack contains the return address. Above that are the parameters passed via the
  // stack.
  // Some of the general purpose and vector registers contain the parameters for the
  // instrumented function not passed via the stack.
  // Compare section "3.2 Function Calling Sequence" in "System V Application Binary Interface"
  // https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf

  // General purpose registers used for passing parameters are rdi, rsi, rdx, rcx, r8, r9
  // in that order. rax is used to indicate the number of vector arguments passed to a function
  // requiring a variable number of arguments. r10 is used for passing a function’s static chain
  // pointer. All of these need to be backed up:
  // push rdi      57
  // push rsi      56
  // push rdx      52
  // push rcx      51
  // push r8       41 50
  // push r9       41 51
  // push rax      50
  // push r10      41 52
  trampoline.AppendBytes({0x57})
      .AppendBytes({0x56})
      .AppendBytes({0x52})
      .AppendBytes({0x51})
      .AppendBytes({0x41, 0x50})
      .AppendBytes({0x41, 0x51})
      .AppendBytes({0x50})
      .AppendBytes({0x41, 0x52});

  // We align the stack to 32 bytes first: round down to a multiple of 32, subtract another 24 and
  // then push 8 byte original rsp. So we are 32 byte aligned after these commands and we can 'pop
  // rsp' later to undo this.
  // mov rax, rsp
  // and rsp, $0xffffffffffffffe0
  // sub rsp, 0x18
  // push rax
  trampoline.AppendBytes({0x48, 0x89, 0xe0})
      .AppendBytes({0x48, 0x83, 0xe4, 0xe0})
      .AppendBytes({0x48, 0x83, 0xec, 0x18})
      .AppendBytes({0x50});

  // Backup vector registers on the stack. They are used to pass float parameters so they need to be
  // preserved. If Avx is supported backup ymm{0,..,8} (which include the xmm{0,..,8} registers as
  // their lower half).
  if (HasAvx()) {
    // sub       esp, 32
    // vmovdqa   (esp), ymm0
    // ...
    // sub       esp, 32
    // vmovdqa   (esp), ymm7
    trampoline.AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x3c, 0x24});
  } else {
    // sub     esp, 16
    // movdqa  (esp), xmm0,
    // ...
    // sub     esp, 16
    // movdqa  (esp), xmm7
    trampoline.AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x0f, 0x7f, 0x3c, 0x24});
  }
}

void AppendPayloadCode(uint64_t payload_address, uint64_t function_address,
                       MachineCode& trampoline) {
  // mov rax, payload_address
  // mov rdi, function_address
  // call rax
  trampoline.AppendBytes({0x48, 0xb8})
      .AppendImmediate64(payload_address)
      .AppendBytes({0x48, 0xbf})
      .AppendImmediate64(function_address)
      .AppendBytes({0xff, 0xd0});
}

void AppendRestoreCode(MachineCode& trampoline) {
  // Restore vector registers (see comment on AppendBackupCode above).
  if (HasAvx()) {
    // vmovdqa   ymm0, (esp)
    // add       esp, 32
    // ...
    // vmovdqa   ymm7, (esp)
    // add       esp, 32
    trampoline.AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x3c, 0x24});
  } else {
    // movdqa   xmm7, (esp)
    // add esp, $0x10
    //...
    // movdqa   xmm0, (esp)
    // add esp, $0x10
    trampoline.AppendBytes({0x66, 0x0f, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10});
  }

  // Undo the 32 byte alignment (see comment on AppendBackupCode above).
  // pop rsp
  trampoline.AppendBytes({0x5c});

  // Restore the general purpose registers (see comment on AppendBackupCode above).
  // pop r10
  // pop rax
  // pop r9
  // pop r8
  // pop rcx
  // pop  rdx
  // pop  rsi
  // pop  rdi
  trampoline.AppendBytes({0x41, 0x5a})
      .AppendBytes({0x58})
      .AppendBytes({0x41, 0x59})
      .AppendBytes({0x41, 0x58})
      .AppendBytes({0x59})
      .AppendBytes({0x5a})
      .AppendBytes({0x5e})
      .AppendBytes({0x5f});
}

struct RelocatedInstruction {
  // Machine code of the relocated instruction. Might contain multiple instructions to emulate what
  // the original instruction acchieved.
  std::vector<uint8_t> code;

  // Some relocated instructions contain an absolute address that needs to be adjusted once all the
  // relocations are done. Example: A conditional jump to a forward position needs to know the
  // position of a instruction not yet processed.
  // Original code does the following:
  // condition cc is true -> InstructionB otherwise -> InstructionA, InstructionB
  //
  // 0x0100: jcc rip+4 (==0x0104)
  // 0x0102: InstructionA
  // 0x0104: InstructionB
  //
  // -> relocate ->
  //
  // 0x0200: j(!cc) rip+10 (== 0x0210)
  // 0x0202: jmp [rip+6] (== [0x0208])
  // 0x0208: 8 byte destination address == address of relocated InstructionB == 0x0217
  // 0x0210: InstructionA'
  // 0x0217: InstructionB'
  //
  // The conditional jump at 0x0100 is translated into the first three lines of the result. The
  // address (at 0x0208) of InstructionB' is not yet known at the point of the translation. So it
  // needs to be recorded and handled later. In this case the `position_of_absolute_address` below
  // would be 8.
  std::optional<size_t> position_of_absolute_address = std::nullopt;
};

ErrorMessageOr<RelocatedInstruction> RelocateInstruction(cs_insn* instruction, uint64_t old_address,
                                                         uint64_t new_address) {
  RelocatedInstruction result;
  if ((instruction->detail->x86.modrm & 0xC7) == 0x05) {
    // The modrm byte can encode a memory operand as a signed 32 bit displacement on the rip. See
    // "Intel 64 and IA-32 Architectures Software Developer’s Manual Vol. 2A" Chapter 2.1.
    // Specifically Table 2-2.
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
    // Jump to relative immediate parameter (32 bit or 8 bit).
    // We compute the absolute address and jump there:
    // jmp [rip + 6]                ff 25 00 00 00 00
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
    // We stored the non-relocated address in the instruction above. In case it belongs to an
    // instruction that was relocated this will be taken care of later:
    result.position_of_absolute_address = 6;
  } else if (instruction->detail->x86.opcode[0] == 0xe8) {
    // Call function at relative immediate parameter.
    // We compute the absolute address of the called function and call it like this:
    // Call [rip+8]                 ff 15 02 00 00 00
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
  } else if ((instruction->detail->x86.opcode[0] & 0xf0) == 0x70) {
    // 0x7? are conditional jumps to an 8 bit immediate.
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
  } else if ((instruction->detail->x86.opcode[0] & 0xfc) == 0xe0) {
    // 0xe{0, 1, 2, 3} loops to an 8 bit immediate.
    // const int8_t immediate =
    //     *absl::bit_cast<int8_t*>(instruction->bytes +
    //     instruction->detail->x86.encoding.imm_offset);
    // const uint64_t absolute_address = old_address + instruction->size + immediate;
    // // We only support loops happening entirely in the overwritten instructions.
    // if (absolute_address<old_address || absolute_address> old_address+kSizeOfJmp) {
    //   return ErrorMessage(absl::StrFormat(
    //       "While trying to relocate a loop instruction . old address: %#x,  instruction: %s",
    //       old_address, new_address, InstructionBytesAsString(instruction)));

    // These instructions are not used by modern compilers as it seems. However we should implement
    // something eventually.
    return ErrorMessage(
        absl::StrFormat("Relocating a loop instruction is not supported. instruction: %s",
                        InstructionBytesAsString(instruction)));

  } else if (instruction->detail->x86.opcode[0] == 0x0f &&
             (instruction->detail->x86.opcode[1] & 0xf0) == 0x80) {
    // 0x0f 0x8? are conditional jumps to a 32 bit immediate
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
  } else {
    // All other instructions can just be copied.
    result.code.resize(instruction->size);
    memcpy(result.code.data(), instruction->bytes, instruction->size);
  }

  return result;
}

// Relocates instructions beginning at `function_address` into the trampoline until `kSizeOfJmp`
// bytes at the beginning of the function are cleared.
// Returns a mapping from old instruction start addresses in the function to new addresses in the
// trampoline. The map is meant to be used to move instruction pointers inside the overwritten areas
// into the correct positions in the trampoline. Therefore only the instructions after the first one
// are included (function_address will contain a valid instruction - the jump into the trampoline -
// when we are done).
ErrorMessageOr<void> AppendRelocatedPrologCode(
    uint64_t function_address, const std::vector<uint8_t>& function, uint64_t trampoline_address,
    csh capstone_handle, uint64_t& address_after_prolog,
    absl::flat_hash_map<uint64_t, uint64_t>& global_relocation_map, MachineCode& trampoline) {
  cs_insn* instruction = cs_malloc(capstone_handle);
  CHECK(instruction != nullptr);
  orbit_base::unique_resource scope_exit{instruction,
                                         [](cs_insn* instruction) { cs_free(instruction, 1); }};
  std::vector<uint8_t> trampoline_code;
  const uint8_t* code_pointer = function.data();
  size_t code_size = function.size();
  uint64_t disassemble_address = function_address;
  std::vector<size_t> relocateable_addresses;
  absl::flat_hash_map<uint64_t, uint64_t> relocation_map;
  while ((disassemble_address - function_address < kSizeOfJmp) &&
         cs_disasm_iter(capstone_handle, &code_pointer, &code_size, &disassemble_address,
                        instruction)) {
    const uint64_t original_instruction_address = disassemble_address - instruction->size;
    const uint64_t relocated_instruction_address =
        trampoline_address + trampoline.GetResultAsVector().size() + trampoline_code.size();
    relocation_map.insert_or_assign(original_instruction_address, relocated_instruction_address);
    OUTCOME_TRY(relocated_instruction,
                RelocateInstruction(instruction, original_instruction_address,
                                    relocated_instruction_address));
    if (relocated_instruction.position_of_absolute_address.has_value()) {
      const size_t offset = relocated_instruction.position_of_absolute_address.value();
      const size_t instruction_address = trampoline_code.size();
      relocateable_addresses.push_back(instruction_address + offset);
    }
    trampoline_code.insert(trampoline_code.end(), relocated_instruction.code.begin(),
                           relocated_instruction.code.end());
  }
  // Relocate addresses encoded in the trampoline.
  for (size_t pos : relocateable_addresses) {
    const uint64_t address_in_trampoline = *absl::bit_cast<uint64_t*>(trampoline_code.data() + pos);
    auto it = relocation_map.find(address_in_trampoline);
    if (it != relocation_map.end()) {
      *absl::bit_cast<uint64_t*>(trampoline_code.data() + pos) = it->second;
    }
  }

  trampoline.AppendBytes(trampoline_code);
  global_relocation_map.insert(relocation_map.begin(), relocation_map.end());
  address_after_prolog = disassemble_address;
  return outcome::success();
}

ErrorMessageOr<void> AppendJumpBackCode(uint64_t address_after_prolog, uint64_t trampoline_address,
                                        MachineCode& trampoline) {
  const uint64_t address_after_jmp =
      trampoline_address + trampoline.GetResultAsVector().size() + kSizeOfJmp;
  trampoline.AppendBytes({0xe9});
  ErrorMessageOr<int32_t> new_offset_or_error =
      AddressDifferenceAsInt32(address_after_prolog, address_after_jmp);
  // This should not happen since the trampoline is allocated such that it is located in the +-2GB
  // range of the instrumented code.
  if (new_offset_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat(
        "Unable to jump back to instrumented function since the instrumented function and the "
        "trampoline are more then +-2GB apart. address_after_prolog: %#x trampoline_address: %#x",
        address_after_prolog, trampoline_address));
  }
  trampoline.AppendImmediate32(new_offset_or_error.value());
  return outcome::success();
}

ErrorMessageOr<void> OverwritePrologWithJump(pid_t pid, uint64_t function_address,
                                             uint64_t address_after_prolog,
                                             uint64_t trampoline_address) {
  MachineCode jump;
  jump.AppendBytes({0xe9});
  ErrorMessageOr<int32_t> offset_or_error =
      AddressDifferenceAsInt32(trampoline_address, function_address + kSizeOfJmp);
  // This should not happen since the trampoline is allocated such that it is located in the +-2GB
  // range of the instrumented code.
  if (offset_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat(
        "Unable to jump from instrumented function into trampoline since the locations are more "
        "then +-2GB apart. function_address: %#x trampoline_address: %#x",
        function_address, trampoline_address));
  }
  jump.AppendImmediate32(offset_or_error.value());
  while (jump.GetResultAsVector().size() < address_after_prolog - function_address) {
    jump.AppendBytes({0x90});
  }
  auto write_result_or_error = WriteTraceesMemory(pid, function_address, jump.GetResultAsVector());
  if (write_result_or_error.has_error()) {
    return write_result_or_error.error();
  }
  return outcome::success();
}

// Instruments function at `function_address` by building a trampoline at `trampoline_address` and
// overwriting the beginning of the function in the tracee with a jump into that trampoline. The
// trampoline will call `payload_address` with `function_address` as a parameter.
// `function` contains the beginning of the function (kMaxFunctionPrologBackupSize or less if the
// function shorter).
// `capstone_handle` is a handle to the capstone disassembler library returned by cs_open.
// The function returns an error if it was no possible to instrument the function. For details on
// that see the comments at AppendRelocatedPrologCode. If the function is successful it will insert
// an address pair into `relocation_map` for each instruction it relocated from the beginning of the
// function into the trampoline. (needed for moving instruction pointers away from the overwritten
// bytes at the beginning of the function).
ErrorMessageOr<void> InstrumentFunction(pid_t pid, uint64_t function_address,
                                        const std::vector<uint8_t>& function,
                                        uint64_t trampoline_address, uint64_t payload_address,
                                        csh capstone_handle,
                                        absl::flat_hash_map<uint64_t, uint64_t>& relocation_map) {
  MachineCode trampoline;
  // Add code to backup register state, execute the payload and restore the register state.
  AppendBackupCode(trampoline);
  AppendPayloadCode(payload_address, function_address, trampoline);
  AppendRestoreCode(trampoline);

  // Relocate prolog into trampoline.
  uint64_t address_after_prolog = 0;
  OUTCOME_TRY(AppendRelocatedPrologCode(function_address, function, trampoline_address,
                                        capstone_handle, address_after_prolog, relocation_map,
                                        trampoline));

  // Add code for jump from trampoline back into function.
  OUTCOME_TRY(AppendJumpBackCode(address_after_prolog, trampoline_address, trampoline));

  // Copy trampoline into tracee.
  auto write_result_or_error =
      WriteTraceesMemory(pid, trampoline_address, trampoline.GetResultAsVector());
  if (write_result_or_error.has_error()) {
    return write_result_or_error.error();
  }

  // Overwrite prolog with jump into the trampoline.
  OUTCOME_TRY(
      OverwritePrologWithJump(pid, function_address, address_after_prolog, trampoline_address));

  return outcome::success();
}

AddressRange GetFunctionAddressRangeOrDie(pid_t pid, std::string_view function_name) {
  // Find the address of the code for `DoSomething`.
  auto modules = ReadModules(pid);
  CHECK(!modules.has_error());
  std::string module_file_path;
  AddressRange address_range_code;
  for (const auto& m : modules.value()) {
    if (m.name() == "UserSpaceInstrumentationTests") {
      module_file_path = m.file_path();
      address_range_code.start = m.address_start();
      address_range_code.end = m.address_end();
    }
  }
  auto elf_file = ElfFile::Create(module_file_path);
  CHECK(!elf_file.has_error());
  auto syms = elf_file.value()->LoadSymbolsFromSymtab();
  CHECK(!syms.has_error());
  uint64_t address = 0;
  uint64_t size = 0;
  for (const auto& sym : syms.value().symbol_infos()) {
    if (sym.name() == function_name) {
      address = sym.address() + address_range_code.start - syms.value().load_bias();
      size = sym.size();
    }
  }
  return {address, address + size};
}

void DumpDissasembly(csh handle, const std::vector<u_int8_t>& code, uint64_t start_address) {
  cs_insn* insn = nullptr;
  size_t count = cs_disasm(handle, static_cast<const uint8_t*>(code.data()), code.size(),
                           start_address, 0, &insn);
  size_t j;
  for (j = 0; j < count; j++) {
    std::string machine_code;
    for (int k = 0; k < insn[j].size; k++) {
      machine_code =
          absl::StrCat(machine_code, k == 0 ? absl::StrFormat("%#0.2x", insn[j].bytes[k])
                                            : absl::StrFormat(" %0.2x", insn[j].bytes[k]));
    }
    LOG("0x%llx:\t%-12s %s , %s", insn[j].address, insn[j].mnemonic, insn[j].op_str, machine_code);
  }
  // Print out the next offset, after the last instruction.
  LOG("0x%llx:", insn[j - 1].address + insn[j - 1].size);
  cs_free(insn, count);
}

// The max tempoline size is a compile time constant but we prefer to compute it here since this
// captures every change to the constant caused by a change to the code constructiong the
// trampoline.
uint64_t GetMaxTrampolineSize() {
  MachineCode unused_code;
  AppendBackupCode(unused_code);
  AppendPayloadCode(0 /* payload_address*/, 0 /* function address */, unused_code);
  AppendRestoreCode(unused_code);
  unused_code.AppendBytes(std::vector<uint8_t>(kMaxRelocatedPrologSize, 0));
  auto result =
      AppendJumpBackCode(0 /*address_after_prolog*/, 0 /*trampoline_address*/, unused_code);
  CHECK(!result.has_error());

  // Round up to the next multiple of eight so we get aligned jump targets at the beginning of the
  // each trampoline.
  return ((unused_code.GetResultAsVector().size() + 7) / 8) * 8;
}

TEST(DisassemblerTest, AddressDifferenceAsInt32) {
  // Result of the difference is negative; in the first case it just fits the second case overflows.
  const uint64_t a = 0x6012345612345678;
  const uint64_t b1 = a - std::numeric_limits<int32_t>::min();
  auto result = AddressDifferenceAsInt32(a, b1);
  ASSERT_FALSE(result.has_error());
  EXPECT_EQ(std::numeric_limits<int32_t>::min(), result.value());
  result = AddressDifferenceAsInt32(a, b1 + 1);
  EXPECT_TRUE(result.has_error());

  // Result of the difference is positive; in the first case it just fits the second case overflows.
  const uint64_t b2 = a - std::numeric_limits<int32_t>::max();
  result = AddressDifferenceAsInt32(a, b2);
  ASSERT_FALSE(result.has_error());
  EXPECT_EQ(std::numeric_limits<int32_t>::max(), result.value());
  EXPECT_TRUE(AddressDifferenceAsInt32(a, b2 - 1).has_error());

  // Result of the difference does not even fit into a int64. We handle that gracefully as well.
  const uint64_t c = 0xa012345612345678;
  const uint64_t d = c - 0x9012345612345678;
  result = AddressDifferenceAsInt32(c, d);
  EXPECT_TRUE(result.has_error());
  result = AddressDifferenceAsInt32(d, c);
  EXPECT_TRUE(result.has_error());
}

class RelocateInstructionTest : public testing::Test {
 protected:
  void SetUp() override {
    CHECK(cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle_) == CS_ERR_OK);
    CHECK(cs_option(capstone_handle_, CS_OPT_DETAIL, CS_OPT_ON) == CS_ERR_OK);
    instruction_ = cs_malloc(capstone_handle_);
    CHECK(instruction_ != nullptr);
  }

  void Disassemble(const MachineCode& code) {
    const uint8_t* code_pointer = code.GetResultAsVector().data();
    size_t code_size = code.GetResultAsVector().size();
    uint64_t disassemble_address = 0;
    CHECK(cs_disasm_iter(capstone_handle_, &code_pointer, &code_size, &disassemble_address,
                         instruction_));
  }

  void TearDown() override {
    cs_free(instruction_, 1);
    cs_close(&capstone_handle_);
  }

  cs_insn* instruction_ = nullptr;

 private:
  csh capstone_handle_ = 0;
};

TEST_F(RelocateInstructionTest, RipRelativeAddressing) {
  MachineCode code;
  constexpr int32_t kOffset = 0x969433;
  // add qword ptr [rip + kOffset], 1
  code.AppendBytes({0x48, 0x83, 0x05}).AppendImmediate32(kOffset).AppendBytes({0x01});
  Disassemble(code);

  uint64_t kOriginalAddress = 0x0100000000;
  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset - 0x123456);
  EXPECT_THAT(result, HasValue());
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x48, 0x83, 0x05, 0x56, 0x34, 0x12, 0x00, 0x01}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

  result =
      RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset - 0x12345678);
  ASSERT_TRUE(result.has_value());
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x48, 0x83, 0x05, 0x78, 0x56, 0x34, 0x12, 0x01}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

  result =
      RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress + kOffset + 0x123456);
  ASSERT_TRUE(result.has_value());
  // -0x123456 == 0xffedcbaa
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x48, 0x83, 0x05, 0xaa, 0xcb, 0xed, 0xff, 0x01}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());

  result = RelocateInstruction(instruction_, kOriginalAddress, kOriginalAddress - 0x7fff0000);
  EXPECT_THAT(result,
              HasError("While trying to relocate an instruction with rip relative addressing the "
                       "target was out of range from the trampoline."));
}

TEST_F(RelocateInstructionTest, DirectCallRelativeImmediateAddress) {
  MachineCode code;
  constexpr int32_t kOffset = 0x01020304;
  // call [rip + kOffset]
  code.AppendBytes({0xe8}).AppendImmediate32(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_TRUE(result.has_value());
  // Call [rip + 2]               ff 15 02 00 00 00
  // jmp  [rip + 8]               eb 08
  // absolute_address             09 03 02 01 01 00 00 00
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0xff, 0x15, 0x02, 0x00, 0x00, 0x00, 0xeb, 0x08, 0x09, 0x03, 0x02,
                                0x01, 0x01, 0x00, 0x00, 0x00}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());
}

TEST_F(RelocateInstructionTest, DirectJumpToRelative8BitImmediate) {
  MachineCode code;
  constexpr int8_t kOffset = 0x08;
  // jmp [rip + kOffset]
  code.AppendBytes({0xeb}).AppendImmediate8(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_TRUE(result.has_value());
  // jmp  [rip + 0]               ff 25 00 00 00 00
  // absolute_address             0a 00 00 00 01 00 00 00
  // original jump instruction end on 0x0100000000 + 0x02. Adding kOffset yields 0x010000000a.
  EXPECT_THAT(result.value().code, ElementsAreArray({0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00,
                                                     0x00, 0x00, 0x01, 0x00, 0x00, 0x00}));
  EXPECT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(6, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, DirectJumpToRelative32BitImmediate) {
  MachineCode code;
  constexpr int32_t kOffset = 0x01020304;
  // jmp [rip + kOffset]
  code.AppendBytes({0xe9}).AppendImmediate32(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_TRUE(result.has_value());
  // jmp  [rip + 0]               ff 25 00 00 00 00
  // absolute_address             09 03 02 01 01 00 00 00
  // original jump instruction end on 0x0100000000 + 0x05. Adding kOffset yields 0x0101020309.
  EXPECT_THAT(result.value().code, ElementsAreArray({0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x09, 0x03,
                                                     0x02, 0x01, 0x01, 0x00, 0x00, 0x00}));
  EXPECT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(6, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, ConditionalDirectJumpToRelative8BitImmediate) {
  MachineCode code;
  constexpr int8_t kOffset = 0x40;
  // jno rip + kOffset
  code.AppendBytes({0x71}).AppendImmediate8(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_TRUE(result.has_value());
  // jo rip + 16                  70 0e
  // jmp [rip + 6]                ff 25 00 00 00 00
  // absolute_address             42 00 00 00 01 00 00 00
  // original jump instruction ends on 0x0100000002 + 0x40 (kOffset) == 0x0100000042.
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x70, 0x0e, 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x00,
                                0x00, 0x01, 0x00, 0x00, 0x00}));
  EXPECT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(8, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, LoopIsUnsupported) {
  MachineCode code;
  constexpr int8_t kOffset = 0x40;
  // loopz rip + kOffset
  code.AppendBytes({0xe1}).AppendImmediate8(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  EXPECT_THAT(result, HasError("Relocating a loop instruction is not supported."));
}

TEST_F(RelocateInstructionTest, ConditionalDirectJumpToRelative32BitImmediate) {
  MachineCode code;
  constexpr int32_t kOffset = 0x12345678;
  // jno rip + kOffset           0f 80 78 56 34 12
  code.AppendBytes({0x0f, 0x80}).AppendImmediate32(kOffset);
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  ASSERT_TRUE(result.has_value());
  // jo rip + 16                  71 0e
  // jmp [rip +6]                 ff 25 00 00 00 00
  // absolute_address             7a 56 34 12 01 00 00 00
  // original jump instruction ends on 0x0100000006 + 0x12345678 (kOffset) == 0x011234567e.
  EXPECT_THAT(result.value().code,
              ElementsAreArray({0x71, 0x0e, 0xff, 0x25, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x56, 0x34,
                                0x12, 0x01, 0x00, 0x00, 0x00}));
  EXPECT_TRUE(result.value().position_of_absolute_address.has_value());
  EXPECT_EQ(8, result.value().position_of_absolute_address.value());
}

TEST_F(RelocateInstructionTest, TrivialTranslation) {
  MachineCode code;
  // nop
  code.AppendBytes({0x90});
  Disassemble(code);

  ErrorMessageOr<RelocatedInstruction> result =
      RelocateInstruction(instruction_, 0x0100000000, 0x0200000000);
  EXPECT_THAT(result.value().code, ElementsAreArray({0x90}));
  EXPECT_FALSE(result.value().position_of_absolute_address.has_value());
}

// TEST(DisassemblerTest, Relocate) {
//   pid_t pid = fork();
//   CHECK(pid != -1);
//   if (pid == 0) {
//     uint64_t sum = 0;
//     int i = 0;
//     while (true) {
//       i = (i + 1) & 3;
//       sum += DoSomething(i);
//     }
//   }

//   // Init Capstone disassembler.
//   csh capstone_handle = 0;
//   cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
//   CHECK(error_code == CS_ERR_OK);
//   error_code = cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
//   CHECK(error_code == CS_ERR_OK);

//   CHECK(AttachAndStopProcess(pid).has_value());

//   // const AddressRange address_range_code =
//   //     GetFunctionAddressRangeOrDie(pid, "_GLOBAL__sub_I_FindFunctionAddressTest.cpp");
//   const AddressRange address_range_code = GetFunctionAddressRangeOrDie(pid, "DoSomething");
//   const uint64_t address_of_do_something = address_range_code.start;
//   const uint64_t size_of_do_something = address_range_code.end - address_range_code.start;
//   LOG("address_of_do_something: %x size_of_do_something: %x", address_of_do_something,
//       size_of_do_something);

//   // Copy the start of the function over into this process.
//   ErrorMessageOr<std::vector<uint8_t>> function_backup =
//       ReadTraceesMemory(pid, address_of_do_something, size_of_do_something);
//   CHECK(function_backup.has_value());

//   {
//     cs_insn* instruction = cs_malloc(capstone_handle);
//     CHECK(instruction != nullptr);
//     orbit_base::unique_resource scope_exit{instruction,
//                                            [](cs_insn* instruction) { cs_free(instruction, 1);
//                                            }};

//     const uint8_t* code_pointer = function_backup.value().data();
//     size_t code_size = size_of_do_something;
//     uint64_t disassemble_address = address_of_do_something;
//     while (cs_disasm_iter(capstone_handle, &code_pointer, &code_size, &disassemble_address,
//                           instruction)) {
//       auto result = RelocateInstruction(instruction, disassemble_address - instruction->size,
//                                         disassemble_address - instruction->size + 0x100000);
//       CHECK(!result.has_error());
//     }
//   }

//   DumpDissasembly(capstone_handle, function_backup.value(), address_of_do_something);

//   cs_close(&capstone_handle);

//   // Detach and end child.
//   CHECK(!DetachAndContinueProcess(pid).has_error());
//   kill(pid, SIGKILL);
//   waitpid(pid, NULL, 0);
// }

TEST(DisassemblerTest, Disassemble) {
  pid_t pid = fork();
  CHECK(pid != -1);
  if (pid == 0) {
    uint64_t sum = 0;
    int i = 0;
    while (true) {
      i = (i + 1) & 3;
      sum += DoSomething(i);
    }
  }

  const uint64_t max_trampoline_size = GetMaxTrampolineSize();

  // Stop the child process using our tooling.
  CHECK(AttachAndStopProcess(pid).has_value());

  // Get address of the function to instrument.
  const AddressRange address_range_code = GetFunctionAddressRangeOrDie(pid, "DoSomething");
  const uint64_t address_of_do_something = address_range_code.start;
  const uint64_t size_of_do_something = address_range_code.end - address_range_code.start;

  // Inject the payload for the instrumentation - just some trivial logging in this case.
  const std::string kLibName = "libUserSpaceInstrumentationTestLib.so";
  const std::string library_path = orbit_base::GetExecutableDir() / ".." / "lib" / kLibName;
  auto library_handle_or_error = DlopenInTracee(pid, library_path, RTLD_NOW);
  CHECK(library_handle_or_error.has_value());
  void* library_handle = library_handle_or_error.value();
  auto logging_function_address_or_error = DlsymInTracee(pid, library_handle, "TrivialLog");
  ASSERT_THAT(logging_function_address_or_error, HasValue());
  uint64_t logging_function_address =
      absl::bit_cast<uint64_t>(logging_function_address_or_error.value());

  // Copy the start of the function 'DoSomething' over into this process.
  absl::flat_hash_map<uint64_t, std::vector<uint8_t>> functions;
  const uint64_t bytes_to_copy = std::min(size_of_do_something, kMaxFunctionPrologBackupSize);
  ErrorMessageOr<std::vector<uint8_t>> function_backup =
      ReadTraceesMemory(pid, address_of_do_something, bytes_to_copy);
  CHECK(function_backup.has_value());
  functions.insert_or_assign(address_of_do_something, function_backup.value());

  // Get memory for the trampolines, well trampoline, we only have one here.
  auto trampoline_or_error =
      AllocateMemoryForTrampolines(pid, address_range_code, max_trampoline_size);
  CHECK(!trampoline_or_error.has_error());
  const uint64_t trampoline_address = trampoline_or_error.value();

  // Init Capstone disassembler.
  csh capstone_handle = 0;
  cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
  CHECK(error_code == CS_ERR_OK);
  //   if (error_code != CS_ERR_OK) {
  //     return ErrorMessage("Unable to open capstone library. Error code: %u", error_code);
  //   }
  error_code = cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
  CHECK(error_code == CS_ERR_OK);

  // Will contain the rip relocations we need to do.
  absl::flat_hash_map<uint64_t, uint64_t> relocation_map;

  // Instrument DoSomething.
  ErrorMessageOr<void> result = InstrumentFunction(
      pid, functions.begin()->first, functions.begin()->second, trampoline_address,
      logging_function_address, capstone_handle, relocation_map);

  // Move every instruction pointer that was in the middle of an overwritten function prolog to the
  // corresponding place in the trampoline.
  std::vector<pid_t> tids = orbit_base::GetTidsOfProcess(pid);
  for (pid_t tid : tids) {
    RegisterState registers;
    CHECK(!registers.BackupRegisters(tid).has_error());
    const uint64_t rip = registers.GetGeneralPurposeRegisters()->x86_64.rip;
    auto relocation = relocation_map.find(rip);
    if (relocation != relocation_map.end()) {
      LOG("Move rip of thread [%d]: %#x -> %#x", tid, rip, relocation->second);
      registers.GetGeneralPurposeRegisters()->x86_64.rip = relocation->second;
      CHECK(!registers.RestoreRegisters().has_error());
    }
  }

  // DEBUG ------------------
  // Disassemble the function, overwritten function and trampoline.
  LOG("original function\n");
  DumpDissasembly(capstone_handle, function_backup.value(), address_of_do_something);

  auto overwritten_function = ReadTraceesMemory(pid, address_of_do_something, bytes_to_copy);
  LOG("\noverwritten function\n");
  DumpDissasembly(capstone_handle, overwritten_function.value(), address_of_do_something);

  auto trampoline = ReadTraceesMemory(pid, trampoline_address, max_trampoline_size);
  LOG("\ntrampoline\n");
  LOG("\nmax_trampoline_size: %u\n", max_trampoline_size);
  DumpDissasembly(capstone_handle, trampoline.value(), trampoline_address);
  // DEBUG ------------------

  cs_close(&capstone_handle);

  // Restart the tracee briefly to assert the thing is still running.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  CHECK(AttachAndStopProcess(pid).has_value());

  // Remove the instrumentation (restore the function prologs, unload the payload library and
  // deallocate the trampolines).
  // The first part is relatively simple: overwrite the instrumented functions with the backed up
  // original version. Since no threads can be executing the overwritten part (they are either on
  // the jump or at a position behind the overwritten bytes) we can just write here.

  // The problem here is that we don't know if the execution of a thread is still stuck in a
  // payload. We can check for instruction pointers in trampolines and the payload library (and this
  // is surprisingly stable) but in theory a thread can be executing code in different module and
  // return to the payload later. In that case we would delete the trampoline and the lib and the
  // thread would segfault later.

  // A stable solution would require to either:
  //   * add bookkeeping into the instrumentation to verify all the threads have left the building
  //   * keep trampoline and payload in the process space forever.
  //   * make sure the payload is *entirely* statically linked. So the heuristic described above
  //   would be a proper guarantee.
  // The first solution comes with a runtime overhead and additional complexity. The second solution
  // is a memory leak: the payload is not a problem since it is not changing but the trampolines
  // would be written again and again (using our current system).
  // We could come up with a system to recycle the trampolines - if there already is one from a
  // previous run we should use that one (might also be a performance benefit).
  // Or we can delete old trampolines after a given time span (if a thread doen't finish to execute
  // the payload after a minute we have more serious problems anyway.)
  // TODO: The third solution is ...

  {
    for (const auto& f : functions) {
      auto write_result_or_error = WriteTraceesMemory(pid, f.first, f.second);
      CHECK(!write_result_or_error.has_error());
    }

    auto modules = ReadModules(pid);
    CHECK(!modules.has_error());
    AddressRange address_range_payload_lib;
    for (const auto& m : modules.value()) {
      if (m.name() == "libUserSpaceInstrumentationTestLib") {
        address_range_payload_lib.start = m.address_start();
        address_range_payload_lib.end = m.address_end();
        break;
      }
    }
    LOG("address_range_payload_lib: %#x - %#x", address_range_payload_lib.start,
        address_range_payload_lib.end);

    bool thread_in_trampoline_or_payload = false;
    do {
      CHECK(!DetachAndContinueProcess(pid).has_error());
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
      CHECK(AttachAndStopProcess(pid).has_value());

      thread_in_trampoline_or_payload = false;
      std::vector<pid_t> tids = orbit_base::GetTidsOfProcess(pid);
      for (pid_t tid : tids) {
        RegisterState registers;
        CHECK(!registers.BackupRegisters(tid).has_error());
        const uint64_t rip = registers.GetGeneralPurposeRegisters()->x86_64.rip;
        // Check for rip in the trampoline.
        if (rip >= trampoline_address && rip <= trampoline_address + max_trampoline_size) {
          thread_in_trampoline_or_payload = true;
          LOG("rip in trampoline");
        }
        // Check for rip in the
        if (rip >= address_range_payload_lib.start && rip <= address_range_payload_lib.end) {
          thread_in_trampoline_or_payload = true;
          LOG("rip in payload");
        }
      }
    } while (thread_in_trampoline_or_payload);

    CHECK(!DlcloseInTracee(pid, library_handle).has_error());
    CHECK(!FreeInTracee(pid, trampoline_address, max_trampoline_size).has_error());
  }
  // Restart the tracee briefly to assert the thing is still running.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  CHECK(AttachAndStopProcess(pid).has_value());

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

// void TrivialLog(uint64_t function_address) {
//   using std::chrono::system_clock;
//   constexpr std::chrono::duration<int, std::ratio<1>> kOneSecond(1);
//   constexpr std::chrono::duration<int, std::ratio<1, 1000>> kThreeMilliseconds(3);
//   static system_clock::time_point last_logged_event = system_clock::now() - kOneSecond;
//   static uint64_t skipped = 0;
//   // Rate limit log output to once every three milliseconds.
//   const system_clock::time_point now = system_clock::now();
//   if (now - last_logged_event > kThreeMilliseconds) {
//     if (skipped > 0) {
//       printf(" ( %lu skipped events )\n", skipped);
//     }
//     printf("Called function at %#lx\n", function_address);
//     last_logged_event = now;
//     skipped = 0;
//   } else {
//     skipped++;
//   }
// }

// void asd() {
//   void (*rax)(uint64_t) = &TrivialLog;
//   uint64_t param = 0xdeadbeef;

//   __asm__ __volatile__(
//       "mov %%rsp, %%rax\n\t"
//       "and $0xffffffffffffffe0, %%rsp\n\t"
//       "sub $0x18, %%rsp\n\t"
//       "push %%rax\n\t"
//       // "sub $0x20, %%rsp\n\t"
//       "mov (%0), %%rax\n\t"
//       "mov (%1), %%rdi\n\t"
//       "call *%%rax\n\t"
//       // "add $0x20, %%rsp\n\t"
//       "pop %%rsp\n\t"
//       :
//       : "r"(&rax), "r"(&param)
//       : "%rax", "memory");
// }

// TEST(AssemblerTest, TrampolineDirektExecution) {
//   asd();
// }

}  // namespace orbit_user_space_instrumentation