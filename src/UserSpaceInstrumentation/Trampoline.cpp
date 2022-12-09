// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Trampoline.h"

#include <absl/base/casts.h>
#include <absl/hash/hash.h>
#include <absl/meta/type_traits.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <absl/types/span.h>
#include <capstone/capstone.h>
#include <capstone/x86.h>
#include <cpuid.h>
#include <unistd.h>

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <limits>
#include <string>
#include <type_traits>
#include <utility>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "MachineCode.h"
#include "OrbitBase/GetProcessIds.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/UniqueResource.h"
#include "RegisterState.h"
#include "UserSpaceInstrumentation/AddressRange.h"

namespace orbit_user_space_instrumentation {

namespace {

using absl::numbers_internal::safe_strtou64_base;
using orbit_base::ReadFileToString;

// Number of bytes to overwrite at the beginning of the function. Relative jump to a signed 32 bit
// offset looks like this:
// jmp 01020304         e9 04 03 02 01
size_t kSizeOfJmp = 5;

// We relocate at most `kSizeOfJmp` instructions. When relocating for each instruction we are either
// copying that instruction or we add a small sequence of instruction and data (see
// RelocateInstruction below). Per instruction we add at most 16 bytes. So we get this (very
// generous) upper bound.
size_t kMaxRelocatedPrologueSize = kSizeOfJmp * 16;

// This constant is the offset of the function id in the trampolines. Since the id of a function
// changes from one profiling run to the next we need to patch every trampoline with the current id
// before each run. This happens in `InstrumentFunction`. Whenever the code of the trampoline is
// changed this constant needs to be adjusted as well. There is a CHECK in the code below to make
// sure this number is correct.
constexpr uint64_t kOffsetOfFunctionIdInCallToEntryPayload = 178;

[[nodiscard]] std::string InstructionBytesAsString(cs_insn* instruction) {
  std::string result;
  for (int i = 0; i < instruction->size; i++) {
    absl::StrAppend(&result, i == 0 ? absl::StrFormat("%#0.2x", instruction->bytes[i])
                                    : absl::StrFormat(" %0.2x", instruction->bytes[i]));
  }
  return result;
}

[[nodiscard]] bool HasAvx() {
  uint32_t eax = 0;
  uint32_t ebx = 0;
  uint32_t ecx = 0;
  uint32_t edx = 0;
  return (__get_cpuid(0x01, &eax, &ebx, &ecx, &edx) != 0) && ((ecx & bit_AVX) != 0u);
}

[[nodiscard]] std::string BytesAsString(absl::Span<const uint8_t> code) {
  std::string result;
  for (const auto& c : code) {
    result.append(absl::StrFormat("%0.2x ", c));
  }
  return result;
}

// Check if somewhere in the code of `function` there is a (conditional) jump back to the first five
// bytes of the function (which we intend to overwrite with a jump into the trampoline). If so we
// must not instrument the function. Note that the entire function is not necessarily available
// here; we'll just disassemble and check whatever we have. Specifically, we are checking for
// conditional and unconditional jumps to 8 and 32 bit offsets (jumps to 16 bit offsets are only
// available in x86 only, this is for x64 only).
// This is merely a heuristic. There can be other jumps either further down in the function or in
// different places in the same translation unit that target the first five bytes of a function.
// However analysing existing code shows that many of the problematic jumps are in small functions
// that are written in assembly. These are detected by the function below.
bool CheckForRelativeJumpIntoFirstFiveBytes(uint64_t function_address,
                                            absl::Span<const uint8_t> function,
                                            csh capstone_handle) {
  cs_insn* instruction = cs_malloc(capstone_handle);
  ORBIT_FAIL_IF(instruction == nullptr, "Failed to allocate memory for capstone disassembler.");
  orbit_base::unique_resource scope_exit{instruction,
                                         [](cs_insn* instruction) { cs_free(instruction, 1); }};

  const uint8_t* code_pointer = function.data();
  size_t code_size = function.size();
  uint64_t disassemble_address = function_address;

  // Disassemble until we run out of instructions in this function.
  while (cs_disasm_iter(capstone_handle, &code_pointer, &code_size, &disassemble_address,
                        instruction)) {
    if ((instruction->detail->x86.opcode[0] == 0xeb) ||
        ((instruction->detail->x86.opcode[0] & 0xf0) == 0x70)) {
      // 0xeb is an unconditional jump to a 8 bit immediate offset.
      // 0x7? are conditional jumps to a 8 bit immediate offset.
      const int8_t immediate = *absl::bit_cast<int8_t*>(
          instruction->bytes + instruction->detail->x86.encoding.imm_offset);
      const uint64_t jump_target_address = disassemble_address + immediate;
      if (jump_target_address >= function_address &&
          jump_target_address < function_address + kSizeOfJmp) {
        return true;
      }
    } else if ((instruction->detail->x86.opcode[0] == 0xe9) ||
               (instruction->detail->x86.opcode[0] == 0x0f &&
                (instruction->detail->x86.opcode[1] & 0xf0) == 0x80)) {
      // 0xe9 is an unconditional jump to a 32 bit immediate offset.
      // 0x0f 0x8? are conditional jumps to a 32 bit immediate offset.
      const int32_t immediate = *absl::bit_cast<int32_t*>(
          instruction->bytes + instruction->detail->x86.encoding.imm_offset);
      const uint64_t jump_target_address = disassemble_address + immediate;
      if (jump_target_address >= function_address &&
          jump_target_address < function_address + kSizeOfJmp) {
        return true;
      }
    }
  }
  return false;
}

// This code is executed immediately after the control is passed to the instrumented function. The
// top of the stack contains the return address. Above that are the parameters passed via the stack.
// Some registers contain the parameters for the instrumented function not passed via the stack.
// Other registers are expected by the caller to not be modified by the callee ("callee-saved",
// "caller-owned" or "non-volatile" registers). As we are going to call the entry payload function,
// all these registers need to be backed up and then restored.
//
// Primarily, we focus on instrumenting functions that follow the System V calling convention
// (see section "3.2 Function Calling Sequence" in "System V Application Binary Interface" at
// https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf, find later versions at
// https://gitlab.com/x86-psABIs/x86-64-ABI/-/wikis/home). But we also want to consider functions
// that follow the Microsoft x64 calling convention (see
// https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention) or the __vectorcall
// convention, which is just a small extension of the Microsoft x64 (see
// https://docs.microsoft.com/en-us/cpp/cpp/vectorcall). This is for two cases: a Windows binary
// running under Wine; a Linux binary with functions following the Microsoft x64 calling convention,
// e.g., with GCC's and Clang's `__attribute__((ms_abi))`.
//
// We assume that our payload functions strictly follow the System V calling convention, and only
// modify registers that don't need to be preserved across function calls in this calling convention
// ("caller-saved", "callee-owned" or "volatile" registers).

// Therefore, for the case of instrumenting functions following the System V calling convention, we
// only need to back up registers used for parameter passing. These are RDI, RSI, RDX, RCX, R8 and
// R9 in that order; XMM{0..7} for floating point arguments, or YMM{0..7} if AVX is available, which
// include XMM{0..7} as their lower half (see section "3.2.3 Parameter Passing" in, again, "System V
// Application Binary Interface"). We know that the remaining volatile registers can be modified,
// and that non-volatile registers will not be modified.
//
// For the case of instrumenting functions that follow the Microsoft x64 calling convention or the
// __vectorcall convention, we again need to back up registers used for parameters passing. These
// are RCX, RDX, R8 and R9, in that order; XMM{0..4} for floating point arguments; XMM{4,5}, or the
// full YMM{0..5} if AVX is supported, for vector arguments in __vectorcall.
// But this time we also need to back up registers that are non-volatile in the Microsoft x64
// calling convention or in the __vectorcall convention, but are volatile in the System V calling
// convention of the payload functions: these are RDI, RSI and XMM{6..15} (but not the full
// YMM{6..15} as their upper half is volatile).
//
// There are other guarantees from the System V calling convention.
// The CPU must be in x87 state when entering a function. If a function switches to MMX, it is
// required to switch back to x87 before returning or calling another function. We don't use x87 or
// alter its state in the machine code, and if the payload switches to MMX, it is guaranteed to
// switch back before returning.
// The x87 status word register is volatile, while the x87 control word (FPCSR) is non-volatile. The
// status bits of the MMX control and status register (MXCSR) are volatile, while the control bits
// are non-volatile.
// The direction flag DF in the %rFLAGS register must be clear (set to "forward" direction) on
// function entry and return.
// We don't need to do anything here: we don't alter the x87 state nor any of the registers just
// mentioned in the machine code, and the payload functions obey the System V calling convention.
// Also, we don't interfere with the Microsoft x64 or __vectorcall convention, as in these
// conventions the x87 status word, the x87 control word, and the MMX control and status bits have
// the same requirements in terms of volatility.
//
// However, the Microsoft x64 calling convention, and the __vectorcall convention that derives from
// it, states
// (https://docs.microsoft.com/en-us/cpp/build/x64-calling-convention#callercallee-saved-registers):
// "The x64 ABI considers the registers RAX, RCX, RDX, R8, R9, R10, R11, and XMM0-XMM5 volatile.
// When present, the upper portions of YMM0-YMM15 [...] are also volatile. [...] Consider volatile
// registers destroyed on function calls **unless otherwise safety-provable by analysis such as
// whole program optimization**."
// Compilers seem to make use of this possibility for optimization. For example, despite them being
// volatile, we have observed R8, R9, R10, R11 being considered preserved across specific function
// calls by some callers, i.e., being written before a function call and read after a function call
// without backing up and restoring the content. Therefore, we decide to back up all registers that
// are volatile in the Microsoft x64 calling convention, regardless of whether they are used for
// parameter passing.
// In the end, this results in backing up almost all registers that are volatile in the System V
// calling convention, except for: the x87 registers ST{0..7}, which also include the MMX mm{0..7}
// registers as their lower 8 bytes; the x87 status word; the status bits of MXCSR. We ignore these:
// the Microsoft x64 and __vectorcall conventions call these "floating-point support for older code"
// and states that "there's no explicit calling convention for" the MM{0..7}/ST{0..7} registers.
//
// Similarly, and more worryingly, the System V calling convention also states: "The standard
// calling sequence requirements apply only to global functions. Local functions that are not
// reachable from other compilation units may use different conventions. Nevertheless, it is
// recommended that all functions use the standard calling sequence when possible."
// For now, we assume that compilers follow the recommendation. Otherwise, we would probably have to
// back up all registers. Note, though, that in practice this would only add RBX, R12, R13, R14 and
// R15 (plus possibly the x87 and MMX registers).
//
// AVX-512 also introduced the ZMM{0..31} registers, to add another 32 bytes to the existing
// YMM{0..15} and add 16 more of them. These are volatile in all the calling conventions we
// consider. We are ignoring these registers for now as AVX-512 is not yet widely used or available,
// but we will probably want to back up the full ZMM{0..7} registers in the future as they can be
// used to pass values of vector type __m512 in the System V calling convention.
//
// In conclusion, we are backing up: RAX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, XMM0-15 (YMM0-15 if
// AVX is available).
void AppendBackupCode(MachineCode& trampoline) {
  // Back up the general purpose registers on the stack.
  //
  // push rax        50
  // push rcx        51
  // push rdx        52
  // push rsi        56
  // push rdi        57
  // push r8         41 50
  // push r9         41 51
  // push r10        41 52
  // push r11        41 53
  trampoline.AppendBytes({0x50})
      .AppendBytes({0x51})
      .AppendBytes({0x52})
      .AppendBytes({0x56})
      .AppendBytes({0x57})
      .AppendBytes({0x41, 0x50})
      .AppendBytes({0x41, 0x51})
      .AppendBytes({0x41, 0x52})
      .AppendBytes({0x41, 0x53});

  // Align the stack to 32 bytes: round down to a multiple of 32, subtract another 24 and then push
  // the original rsp (another 8 bytes). We can 'pop rsp' later to undo this.
  //
  // mov rax, rsp                       48 89 e0
  // and rsp, 0xffffffffffffffe0        48 83 e4 e0
  // sub rsp, 0x18                      48 83 ec 18
  // push rax                           50
  trampoline.AppendBytes({0x48, 0x89, 0xe0})
      .AppendBytes({0x48, 0x83, 0xe4, 0xe0})
      .AppendBytes({0x48, 0x83, 0xec, 0x18})
      .AppendBytes({0x50});

  // Back up the vector registers on the stack. If AVX is supported, back up ymm{0,..,15} (which
  // include the xmm{0,..,15} registers as their lower half).
  if (HasAvx()) {
    // sub       rsp, 32            48 83 ec 20
    // vmovdqa   [rsp], ymm0        c5 fd 7f 04 24
    // ...
    // sub       rsp, 32            48 83 ec 20
    // vmovdqa   [rsp], ymm15       c5 7d 7f 3c 24
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
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x3c, 0x24});
  } else {
    // sub     rsp, 16            48 83 ec 10
    // movdqa  [rsp], xmm0        66 0f 7f 04 24
    // ...
    // sub     rsp, 16            48 83 ec 10
    // movdqa  [rsp], xmm15       66 44 0f 7f 3c 24
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
        .AppendBytes({0x66, 0x0f, 0x7f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x3c, 0x24});
  }
}

// Call the entry payload function with the return address, the id of the instrumented function,
// the original stack pointer (i.e., address of the return address) and the address of the return
// trampoline as parameters. Note that the stack is still aligned (compare `AppendBackupCode` above)
// as required by the calling convention as per section "3.2.2 The Stack Frame" in, again, "System V
// Application Binary Interface".
void AppendCallToEntryPayload(uint64_t entry_payload_function_address,
                              uint64_t return_trampoline_address, MachineCode& trampoline) {
  // At this point rax is the rsp after pushing the general purpose registers, so adding 0x48 (72)
  // gets us the location of the return address (see above in `AppendBackupCode`).

  // add rax, 0x48                                   48 83 c0 48
  // mov rdi, [rax]                                  48 8b 38
  // mov rsi, function_id                            48 be function_id
  // mov rdx, rax                                    48 89 c2
  // mov rcx, return_trampoline_address              48 b9 return_trampoline_address
  // mov rax, entry_payload_function_address         48 b8 addr
  // call rax                                        ff d0
  trampoline.AppendBytes({0x48, 0x83, 0xc0, 0x48})
      .AppendBytes({0x48, 0x8b, 0x38})
      .AppendBytes({0x48, 0xbe});
  // This fails if the code for the trampoline was changed - see the comment at the declaration of
  // kOffsetOfFunctionIdInCallToEntryPayload above.
  ORBIT_CHECK(trampoline.GetResultAsVector().size() == kOffsetOfFunctionIdInCallToEntryPayload);
  // The value of function id will be overwritten by every call to `InstrumentFunction`. This is
  // just a placeholder.
  trampoline.AppendImmediate64(0xDEADBEEFDEADBEEF)
      .AppendBytes({0x48, 0x89, 0xc2})
      .AppendBytes({0x48, 0xb9})
      .AppendImmediate64(return_trampoline_address)
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(entry_payload_function_address)
      .AppendBytes({0xff, 0xd0});
}

void AppendRestoreCode(MachineCode& trampoline) {
  // Restore the vector registers (see comment on AppendBackupCode above).
  if (HasAvx()) {
    // vmovdqa   ymm15, [rsp]        c5 7d 6f 3c 24
    // add       rsp, 32             48 83 c4 20
    // ...
    // vmovdqa   ymm0, [rsp]         c5 fd 6f 04 24
    // add       rsp, 32             48 83 c4 20
    trampoline.AppendBytes({0xc5, 0x7d, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20});
  } else {
    // movdqa   xmm15, [rsp]       66 44 0f 6f 3c 24
    // add rsp, 0x10               48 83 c4 10
    //...
    // movdqa   xmm0, [rsp]        66 0f 6f 04 24
    // add rsp, 0x10               48 83 c4 10
    trampoline.AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x3c, 0x24})
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

  // Undo the 32-byte alignment (see comment on AppendBackupCode above).
  //
  // pop rsp        5c
  trampoline.AppendBytes({0x5c});

  // Restore the general purpose registers (see comment on AppendBackupCode above).
  // pop r11        41 5b
  // pop r10        41 5a
  // pop r9         41 59
  // pop r8         41 58
  // pop rdi        5f
  // pop rsi        5e
  // pop rdx        5a
  // pop rcx        59
  // pop rax        58
  trampoline.AppendBytes({0x41, 0x5b})
      .AppendBytes({0x41, 0x5a})
      .AppendBytes({0x41, 0x59})
      .AppendBytes({0x41, 0x58})
      .AppendBytes({0x5f})
      .AppendBytes({0x5e})
      .AppendBytes({0x5a})
      .AppendBytes({0x59})
      .AppendBytes({0x58});
}

// Relocates instructions beginning at `function_address` into the trampoline until `kSizeOfJmp`
// bytes at the beginning of the function are cleared.
// Returns a mapping from old instruction start addresses in the function to new addresses in the
// trampoline. The map is meant to be used to move instruction pointers inside the overwritten areas
// into the correct positions in the trampoline. Therefore, only the instructions after the first
// one are included (function_address will contain a valid instruction - the jump into the
// trampoline - when we are done).
[[nodiscard]] ErrorMessageOr<uint64_t> AppendRelocatedPrologueCode(
    uint64_t function_address, absl::Span<const uint8_t> function, uint64_t trampoline_address,
    csh capstone_handle, absl::flat_hash_map<uint64_t, uint64_t>& global_relocation_map,
    MachineCode& trampoline) {
  cs_insn* instruction = cs_malloc(capstone_handle);
  ORBIT_FAIL_IF(instruction == nullptr, "Failed to allocate memory for capstone disassembler.");
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
    OUTCOME_TRY(auto&& relocated_instruction,
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

  if (disassemble_address - function_address < kSizeOfJmp) {
    return ErrorMessage(
        absl::StrFormat("Unable to disassemble enough of the function to instrument it. Code: %s",
                        BytesAsString(function)));
  }

  // Relocate addresses encoded in the trampoline.
  for (size_t pos : relocateable_addresses) {
    uint64_t address_in_trampoline{};
    std::memcpy(&address_in_trampoline, trampoline_code.data() + pos,
                sizeof(address_in_trampoline));
    auto it = relocation_map.find(address_in_trampoline);
    if (it != relocation_map.end()) {
      std::memcpy(trampoline_code.data() + pos, &it->second, sizeof(it->second));
    }
  }

  trampoline.AppendBytes(trampoline_code);
  global_relocation_map.insert(relocation_map.begin(), relocation_map.end());
  return disassemble_address;
}

[[nodiscard]] ErrorMessageOr<void> AppendJumpBackCode(uint64_t address_after_prologue,
                                                      uint64_t trampoline_address,
                                                      MachineCode& trampoline) {
  const uint64_t address_after_jmp =
      trampoline_address + trampoline.GetResultAsVector().size() + kSizeOfJmp;
  ErrorMessageOr<int32_t> new_offset_or_error =
      AddressDifferenceAsInt32(address_after_prologue, address_after_jmp);
  // This should not happen since the trampoline is allocated such that it is located in the +-2GB
  // range of the instrumented code.
  if (new_offset_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat(
        "Unable to jump back to instrumented function since the instrumented function and the "
        "trampoline are more then +-2GB apart. address_after_prologue: %#x trampoline_address: %#x",
        address_after_prologue, trampoline_address));
  }
  // jmp new_offset        e9 off
  trampoline.AppendBytes({0xe9}).AppendImmediate32(new_offset_or_error.value());
  return outcome::success();
}

// This code is executed immediately after the instrumented function has returned. We are going to
// call the exit payload function, so we need to preserve the registers that might be used to return
// values, and all registers that the payload might modify even if they are non-volatile in other
// calling conventions.
//
// In particular, the System V calling convention returns values in RAX, RDX, XMM0, XMM1, ST0, ST1.
// The Microsoft x64 calling convention returns values in RAX and XMM0.
// The __vectorcall convention returns values in RAX and XMM{0..3} (YMM{0..3} if AVX is available).
// In addition to the registers just mentioned, we also need to back up registers that are
// non-volatile in the Microsoft x64 or __vectorcall conventions, but are volatile in the System V
// calling convention.
//
// Almost everything that was said for the entry trampoline (see comment on AppendBackupCode above)
// applies unchanged here. This includes backing up (almost) all registers that are volatile in the
// Microsoft x64 or __vectorcall conventions.
//
// There is an additional detail, though: the top two elements of the x87 FPU register stack, ST0
// and ST1 (we mentioned them above), are used in the System V calling convention to return `long
// double` values (in ST0) and C99's `complex long double` values (real part in ST0, imaginary part
// in ST1). We should back up and restore them, but only when they are actually used, as the calling
// convention also requires us to leave the x87 FPU register stack empty when leaving a function
// ("System V Application Binary Interface" for 64 bit is not clear on this, but this can be found
// for 32 bit ,and we believe it carried over). This is not easy with minimal overhead, so we make
// an exception: we simply decide not to back those registers up as the ExitPayload will not use
// them anyway. More details in b/224446632.
//
// In conclusion, in the return trampoline we are backing up the same registers as in the entry
// trampoline: RAX, RCX, RDX, RSI, RDI, R8, R9, R10, R11, ST0, ST1, XMM0-15 (YMM0-15 if AVX is
// available).
//
// After having backed up these registers, called the exit payload, and restored the backed up
// registers, we return to the actual return address, returned by the exit payload.
void AppendCallToExitPayloadAndJumpToReturnAddress(uint64_t exit_payload_function_address,
                                                   MachineCode& return_trampoline) {
  // Make space to store the original return address returned by the ExitPayload.
  //
  // sub rsp, 8        48 83 ec 08
  return_trampoline.AppendBytes({0x48, 0x83, 0xec, 0x08});

  // Back up the general purpose registers.
  //
  // push rax        50
  // push rcx        51
  // push rdx        52
  // push rsi        56
  // push rdi        57
  // push r8         41 50
  // push r9         41 51
  // push r10        41 52
  // push r11        41 53
  return_trampoline.AppendBytes({0x50})
      .AppendBytes({0x51})
      .AppendBytes({0x52})
      .AppendBytes({0x56})
      .AppendBytes({0x57})
      .AppendBytes({0x41, 0x50})
      .AppendBytes({0x41, 0x51})
      .AppendBytes({0x41, 0x52})
      .AppendBytes({0x41, 0x53});

  // Align the stack to 32 bytes: round down to a multiple of 32, subtract another 24 and then push
  // the original rsp (another 8 bytes). We can 'pop rsp' later to undo this.
  //
  // mov rax, rsp                       48 89 e0
  // and rsp, 0xffffffffffffffe0        48 83 e4 e0
  // sub rsp, 0x18                      48 83 ec 18
  // push rax                           50
  return_trampoline.AppendBytes({0x48, 0x89, 0xe0})
      .AppendBytes({0x48, 0x83, 0xe4, 0xe0})
      .AppendBytes({0x48, 0x83, 0xec, 0x18})
      .AppendBytes({0x50});

  // Back up the vector registers on the stack. If AVX is supported backup ymm{0,..,15} (which
  // include the xmm{0,..,15} registers as their lower half).
  if (HasAvx()) {
    // sub       rsp, 32            48 83 ec 20
    // vmovdqa   [rsp], ymm0        c5 fd 7f 04 24
    // ...
    // sub       rsp, 32            48 83 ec 20
    // vmovdqa   [rsp], ymm15       c5 7d 7f 3c 24
    return_trampoline.AppendBytes({0x48, 0x83, 0xec, 0x20})
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
        .AppendBytes({0xc5, 0xfd, 0x7f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x7f, 0x3c, 0x24});
  } else {
    // sub     rsp, 16            48 83 ec 10
    // movdqa  [rsp], xmm0        66 0f 7f 04 24
    // ...
    // sub     rsp, 16            48 83 ec 10
    // movdqa  [rsp], xmm15       66 44 0f 7f 3c 24
    return_trampoline.AppendBytes({0x48, 0x83, 0xec, 0x10})
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
        .AppendBytes({0x66, 0x0f, 0x7f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xec, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x7f, 0x3c, 0x24});
  }

  // At this point, rax is the rsp after pushing nine general purpose registers, so adding 72 gets
  // us the location where we want to store the original return address. Back up this value.
  //
  // add rax, 72        48 83 c0 48
  // push rax           50
  return_trampoline.AppendBytes({0x48, 0x83, 0xc0, 0x48}).AppendBytes({0x50});
  // Now call ExitPayload.
  // rsp was 32-byte aligned before the push, so we need to realign to 16 bytes before calling
  // ExitPayload, as required by both the System V calling convention (see section "3.2.2 The Stack
  // Frame" in, again, "System V Application Binary Interface") and the Microsoft x64 and
  // __vectorcall conventions (see
  // https://docs.microsoft.com/en-us/cpp/build/stack-usage#stack-allocation):
  //
  // sub rsp, 0x08                                 48 83 ec 08
  // mov rax, exit_payload_function_address        48 b8 addr
  // call rax                                      ff d0
  return_trampoline.AppendBytes({0x48, 0x83, 0xec, 0x08})
      .AppendBytes({0x48, 0xb8})
      .AppendImmediate64(exit_payload_function_address)
      .AppendBytes({0xff, 0xd0});
  // The original return address is now in rax. Place it in the desired location, which we recover
  // from the stack and temporarily place in rcx.
  //
  // add rsp, 0x08         48 83 c4 08
  // pop rcx               59
  // mov [rcx], rax        48 89 01
  return_trampoline.AppendBytes({0x48, 0x83, 0xc4, 0x08})
      .AppendBytes({0x59})
      .AppendBytes({0x48, 0x89, 0x01});

  // Restore the vector registers.
  if (HasAvx()) {
    // vmovdqa   ymm15, [rsp]        c5 7d 6f 3c 24
    // add       rsp, 32             48 83 c4 20
    // ...
    // vmovdqa   ymm0, [rsp]         c5 fd 6f 04 24
    // add       rsp, 32             48 83 c4 20
    return_trampoline.AppendBytes({0xc5, 0x7d, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0x7d, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20})
        .AppendBytes({0xc5, 0xfd, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x20});
  } else {
    // movdqa   xmm15, [rsp]       66 44 0f 6f 3c 24
    // add rsp, 0x10               48 83 c4 10
    //...
    // movdqa   xmm0, [rsp]        66 0f 6f 04 24
    // add rsp, 0x10               48 83 c4 10
    return_trampoline.AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x3c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x34, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x2c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x24, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x1c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x14, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x0c, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x44, 0x0f, 0x6f, 0x04, 0x24})
        .AppendBytes({0x48, 0x83, 0xc4, 0x10})
        .AppendBytes({0x66, 0x0f, 0x6f, 0x3c, 0x24})
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

  // Undo the 32-byte alignment.
  //
  // pop rsp        5c
  return_trampoline.AppendBytes({0x5c});

  // Restore the general purpose registers.
  // pop r11        41 5b
  // pop r10        41 5a
  // pop r9         41 59
  // pop r8         41 58
  // pop rdi        5f
  // pop rsi        5e
  // pop rdx        5a
  // pop rcx        59
  // pop rax        58
  return_trampoline.AppendBytes({0x41, 0x5b})
      .AppendBytes({0x41, 0x5a})
      .AppendBytes({0x41, 0x59})
      .AppendBytes({0x41, 0x58})
      .AppendBytes({0x5f})
      .AppendBytes({0x5e})
      .AppendBytes({0x5a})
      .AppendBytes({0x59})
      .AppendBytes({0x58});

  // Return to the actual return address, which is now at the top of the stack.
  // ret        c3
  return_trampoline.AppendBytes({0xc3});
}

}  // namespace

bool DoAddressRangesOverlap(const AddressRange& a, const AddressRange& b) {
  return !(b.end <= a.start || b.start >= a.end);
}

std::optional<size_t> LowestIntersectingAddressRange(absl::Span<const AddressRange> ranges_sorted,
                                                     const AddressRange& range) {
  for (size_t i = 0; i < ranges_sorted.size(); i++) {
    if (DoAddressRangesOverlap(ranges_sorted[i], range)) {
      return i;
    }
  }
  return std::nullopt;
}

std::optional<size_t> HighestIntersectingAddressRange(absl::Span<const AddressRange> ranges_sorted,
                                                      const AddressRange& range) {
  for (int i = ranges_sorted.size() - 1; i >= 0; i--) {
    if (DoAddressRangesOverlap(ranges_sorted[i], range)) {
      return i;
    }
  }
  return std::nullopt;
}

ErrorMessageOr<std::vector<AddressRange>> GetUnavailableAddressRanges(pid_t pid) {
  std::vector<AddressRange> result;
  OUTCOME_TRY(auto&& mmap_min_addr, ReadFileToString("/proc/sys/vm/mmap_min_addr"));
  uint64_t mmap_min_addr_as_uint64 = 0;
  if (!absl::SimpleAtoi(mmap_min_addr, &mmap_min_addr_as_uint64)) {
    return ErrorMessage("Failed to parse /proc/sys/vm/mmap_min_addr");
  }
  result.emplace_back(0, mmap_min_addr_as_uint64);

  OUTCOME_TRY(auto&& maps, ReadFileToString(absl::StrFormat("/proc/%d/maps", pid)));
  std::vector<std::string> lines = absl::StrSplit(maps, '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.empty()) {
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
    ORBIT_CHECK(address_begin < address_end);
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
    absl::Span<const AddressRange> unavailable_ranges, const AddressRange& code_range,
    uint64_t size) {
  constexpr uint64_t kMax32BitOffset = INT32_MAX;
  constexpr uint64_t kMax64BitAddress = UINT64_MAX;
  const uint64_t page_size = sysconf(_SC_PAGE_SIZE);

  ORBIT_FAIL_IF(unavailable_ranges.empty() || unavailable_ranges[0].start != 0,
                "First entry at unavailable_ranges needs to start at zero. Use result of "
                "GetUnavailableAddressRanges.");

  // Try to fit an interval of length `size` below `code_range`.
  auto optional_range_index = LowestIntersectingAddressRange(unavailable_ranges, code_range);
  if (!optional_range_index) {
    return ErrorMessage(absl::StrFormat("code_range %#x-%#x is not in unavailable_ranges.",
                                        code_range.start, code_range.end));
  }
  while (optional_range_index.value() > 0) {
    // Place directly to the left of the taken interval we are in...
    if (unavailable_ranges[optional_range_index.value()].start < size) {
      break;
    }
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
    // Check if we are so close to the end of the address space such that rounding up would
    // overflow.
    if (unavailable_ranges[optional_range_index.value()].end > kMax64BitAddress - (page_size - 1)) {
      break;
    }
    const uint64_t trampoline_address =
        ((unavailable_ranges[optional_range_index.value()].end + (page_size - 1)) / page_size) *
        page_size;
    // Check if we ran out of address space.
    if (trampoline_address >= kMax64BitAddress - size) {
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

ErrorMessageOr<std::unique_ptr<MemoryInTracee>> AllocateMemoryForTrampolines(
    pid_t pid, const AddressRange& code_range, uint64_t size) {
  OUTCOME_TRY(auto&& unavailable_ranges, GetUnavailableAddressRanges(pid));
  OUTCOME_TRY(auto&& address_range,
              FindAddressRangeForTrampoline(unavailable_ranges, code_range, size));
  OUTCOME_TRY(auto&& memory, MemoryInTracee::Create(pid, address_range.start, size));
  return std::move(memory);
}

ErrorMessageOr<int32_t> AddressDifferenceAsInt32(uint64_t a, uint64_t b) {
  constexpr auto kAbsMaxInt32AsUint64 = static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
  constexpr auto kAbsMinInt32AsUint64 =
      static_cast<uint64_t>(-static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
  if ((a > b) && (a - b > kAbsMaxInt32AsUint64)) {
    return ErrorMessage("Difference is larger than +2GB.");
  }
  if ((b > a) && (b - a > kAbsMinInt32AsUint64)) {
    return ErrorMessage("Difference is larger than -2GB.");
  }
  return a - b;
}

ErrorMessageOr<RelocatedInstruction> RelocateInstruction(cs_insn* instruction, uint64_t old_address,
                                                         uint64_t new_address) {
  RelocatedInstruction result;
  if ((instruction->detail->x86.modrm & 0xC7) == 0x05) {
    // The encoding of an x86 instruction contains instruction prefixes, an opcode, the modrm and
    // sib bytes, 1, 2 or 4 bytes address displacement and 1, 2 or 4 bytes of immediate data.
    // Most of these are optional - at least one byte of opcode needs to be present.
    // Many instructions that refer to an operand in memory have an addressing-form specifier byte
    // (called the modrm byte) following the primary opcode.
    // In case (modrm & 0xC7 == 0x05) this modrm byte encodes a memory operand that is computed as
    // the rip of the next instruction plus the 32 bit offset encoded in the four address
    // displacement bytes of the instruction.
    // See "Intel 64 and IA-32 Architectures Software Developer’s Manual Vol. 2A" Chapter 2.1.
    // Specifically table 2-2.
    //
    // Example of original code (add one to memory location at offset 0x123456 from rip):
    // add [rip + 0x123456], 1       48 83 05 56 34 12 00 01
    // The relocated instruction looks the same - we merely adjust the 0x123456 such that we address
    // the same memory location but relative to the new code location.
    const int32_t old_displacement = *absl::bit_cast<int32_t*>(
        instruction->bytes + instruction->detail->x86.encoding.disp_offset);
    const uint64_t old_absolute_address = old_address + instruction->size + old_displacement;
    ErrorMessageOr<int32_t> new_displacement_or_error =
        AddressDifferenceAsInt32(old_absolute_address, new_address + instruction->size);
    if (new_displacement_or_error.has_error()) {
      return ErrorMessage(absl::StrFormat(
          "While trying to relocate an instruction with rip relative addressing the target was out "
          "of range from the trampoline. old address: %#x, new address: %#x, instruction: %s",
          old_address, new_address, InstructionBytesAsString(instruction)));
    }
    result.code.resize(instruction->size);
    memcpy(result.code.data(), instruction->bytes, instruction->size);
    *absl::bit_cast<int32_t*>(result.code.data() + instruction->detail->x86.encoding.disp_offset) =
        new_displacement_or_error.value();
  } else if (instruction->detail->x86.opcode[0] == 0xeb ||
             instruction->detail->x86.opcode[0] == 0xe9) {
    // This handles unconditional jump to relative immediate parameter (32 bit or 8 bit).
    // Example of original code (jump to a 32 bit offset):
    // jmp 0x01020304               e9 01 20 03 04
    // In both cases (8 and 32 bit offsets) we compute the absolute address of the jump target,
    // store it in memory in the trampoline and jump there:
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
    code.AppendBytes({0xff, 0x25}).AppendImmediate32(0).AppendImmediate64(absolute_address);
    result.code = code.GetResultAsVector();
    result.position_of_absolute_address = 6;
  } else if (instruction->detail->x86.opcode[0] == 0xe8) {
    // Call function at relative immediate parameter.
    // Example of original code (call function at offset 0x01020304):
    // call 0x01020304              e8 04 03 02 01
    //
    // We could relocate the call instruction as follows. We compute the absolute address of the
    // called function and call it like this:
    // call [rip+2]                 ff 15 02 00 00 00
    // jmp label;                   eb 08
    // .byte absolute_address       01 02 03 04 05 06 07 08
    // label:
    //
    // But currently we don't want to support relocating a call instruction. Every sample that
    // involves a relocated instruction is an unwinding error. This is normally not a problem for a
    // couple of relocated instructions at the beginning of a function, that would correspond to
    // innermost frames. But for call instructions, an arbitrarily large number of callstacks could
    // be affected, the ones falling in the function and all its tree of callees, and we want to
    // prevent that. Refer to http://b/194704608#comment3.
    return ErrorMessage(
        absl::StrFormat("Relocating a call instruction is not supported. Instruction: %s",
                        InstructionBytesAsString(instruction)));
  } else if ((instruction->detail->x86.opcode[0] & 0xf0) == 0x70) {
    // 0x7? are conditional jumps to an 8 bit immediate.
    // Example of original code (jump backwards 10 bytes if last result was not zero):
    // jne 0xf6                     75 f6
    // We invert the condition of the jump, compute the absolute address of the jump target and
    // construct the following code sequence.
    // je 0x0e                      74 0e  // 0x0e == 14 = 6 bytes jmp + 8 bytes address
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
    // 0x0f 0x8? are conditional jumps to a 32 bit immediate.
    // Example of original code (jump backwards 10 bytes if last result was not zero):
    // jne                          0f 85 f6 ff ff ff
    // We invert the condition of the jump and construct the following code sequence.
    // je 0x0e                      74 0e  // 0x0e == 14 = 6 bytes jmp + 8 bytes address
    // jmp [rip + 0]                ff 25 00 00 00 00
    // .byte absolute_address       01 02 03 04 05 06 07 08
    const int32_t immediate = *absl::bit_cast<int32_t*>(
        instruction->bytes + instruction->detail->x86.encoding.imm_offset);
    const uint64_t absolute_address = old_address + instruction->size + immediate;
    MachineCode code;
    // Inverting the last bit negates the condition for the jump. We need a jump to an eight bit
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

uint64_t GetMaxTrampolineSize() {
  // The maximum size of a trampoline is constant. So the calculation can be cached on first call.
  static const uint64_t trampoline_size = []() -> uint64_t {
    MachineCode unused_code;
    AppendBackupCode(unused_code);
    AppendCallToEntryPayload(/*entry_payload_function_address=*/0,
                             /*return_trampoline_address=*/0, unused_code);
    AppendRestoreCode(unused_code);
    unused_code.AppendBytes(std::vector<uint8_t>(kMaxRelocatedPrologueSize, 0));
    auto result =
        AppendJumpBackCode(/*address_after_prologue=*/0, /*trampoline_address=*/0, unused_code);
    ORBIT_CHECK(!result.has_error());

    // Round up to the next multiple of 32 so we get aligned jump targets at the beginning of the
    // each trampoline.
    return static_cast<uint64_t>(((unused_code.GetResultAsVector().size() + 31) / 32) * 32);
  }();

  return trampoline_size;
}

ErrorMessageOr<uint64_t> CreateTrampoline(pid_t pid, uint64_t function_address,
                                          absl::Span<const uint8_t> function,
                                          uint64_t trampoline_address,
                                          uint64_t entry_payload_function_address,
                                          uint64_t return_trampoline_address, csh capstone_handle,
                                          absl::flat_hash_map<uint64_t, uint64_t>& relocation_map) {
  const bool harmful_jump =
      CheckForRelativeJumpIntoFirstFiveBytes(function_address, function, capstone_handle);
  if (harmful_jump) {
    return ErrorMessage(
        "Failed to create trampoline since the function contains a jump back into the first five "
        "bytes of the function.");
  }

  MachineCode trampoline;
  // Add code to backup register state, execute the payload and restore the register state.
  AppendBackupCode(trampoline);
  AppendCallToEntryPayload(entry_payload_function_address, return_trampoline_address, trampoline);
  AppendRestoreCode(trampoline);

  // Relocate prologue into trampoline.
  OUTCOME_TRY(auto&& address_after_prologue,
              AppendRelocatedPrologueCode(function_address, function, trampoline_address,
                                          capstone_handle, relocation_map, trampoline));

  // Add code for jump from trampoline back into function.
  OUTCOME_TRY(AppendJumpBackCode(address_after_prologue, trampoline_address, trampoline));

  // Copy trampoline into tracee.
  auto write_result_or_error =
      WriteTraceesMemory(pid, trampoline_address, trampoline.GetResultAsVector());
  if (write_result_or_error.has_error()) {
    return write_result_or_error.error();
  }

  return address_after_prologue;
}

uint64_t GetReturnTrampolineSize() {
  // The size is constant. So the calculation can be cached on first call.
  static const uint64_t return_trampoline_size = []() -> uint64_t {
    MachineCode unused_code;
    AppendCallToExitPayloadAndJumpToReturnAddress(/*exit_payload_function_address=*/0, unused_code);
    return static_cast<uint64_t>(((unused_code.GetResultAsVector().size() + 31) / 32) * 32);
  }();

  return return_trampoline_size;
}

ErrorMessageOr<void> CreateReturnTrampoline(pid_t pid, uint64_t exit_payload_function_address,
                                            uint64_t return_trampoline_address) {
  MachineCode return_trampoline;
  AppendCallToExitPayloadAndJumpToReturnAddress(exit_payload_function_address, return_trampoline);

  // Copy into tracee.
  auto write_result_or_error =
      WriteTraceesMemory(pid, return_trampoline_address, return_trampoline.GetResultAsVector());
  if (write_result_or_error.has_error()) {
    return write_result_or_error.error();
  }
  return outcome::success();
}

ErrorMessageOr<void> InstrumentFunction(pid_t pid, uint64_t function_address, uint64_t function_id,
                                        uint64_t address_after_prologue,
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
  // Overwrite the remaining bytes to the next instruction with 'nop's. This is not strictly needed
  // but helps with debugging/disassembling.
  while (jump.GetResultAsVector().size() < address_after_prologue - function_address) {
    jump.AppendBytes({0x90});
  }
  OUTCOME_TRY(WriteTraceesMemory(pid, function_address, jump.GetResultAsVector()));

  // Patch the trampoline to hand over the current function_id to the entry payload.
  MachineCode function_id_as_bytes;
  function_id_as_bytes.AppendImmediate64(function_id);
  OUTCOME_TRY(WriteTraceesMemory(pid, trampoline_address + kOffsetOfFunctionIdInCallToEntryPayload,
                                 function_id_as_bytes.GetResultAsVector()));

  return outcome::success();
}

void MoveInstructionPointersOutOfOverwrittenCode(
    pid_t pid, const absl::flat_hash_map<uint64_t, uint64_t>& relocation_map) {
  std::vector<pid_t> tids = orbit_base::GetTidsOfProcess(pid);
  for (pid_t tid : tids) {
    RegisterState registers;
    ErrorMessageOr<void> backup_or_error = registers.BackupRegisters(tid);
    ORBIT_FAIL_IF(backup_or_error.has_error(),
                  "Failed to read registers in MoveInstructionPointersOutOfOverwrittenCode: %s",
                  backup_or_error.error().message());
    const uint64_t rip = registers.GetGeneralPurposeRegisters()->x86_64.rip;
    auto relocation = relocation_map.find(rip);
    if (relocation != relocation_map.end()) {
      registers.GetGeneralPurposeRegisters()->x86_64.rip = relocation->second;
      ErrorMessageOr<void> restore_or_error = registers.RestoreRegisters();
      ORBIT_FAIL_IF(restore_or_error.has_error(),
                    "Failed to write registers in MoveInstructionPointersOutOfOverwrittenCode: %s",
                    restore_or_error.error().message());
    }
  }
}

}  // namespace orbit_user_space_instrumentation
