// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <capstone/capstone.h>
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
  // To be implemented
  trampoline.AppendBytes({0x90});
}

void AppendPayloadCode(uint64_t payload_address, MachineCode& trampoline) {
  // To be implemented
  LOG("unused: %u", payload_address);
  trampoline.AppendBytes({0x90});
}

void AppendRestoreCode(MachineCode& trampoline) {
  // To be implemented
  trampoline.AppendBytes({0x90});
}

struct RelocatedInstruction {
  std::vector<uint8_t> code;
  // Some relocated instructions contain an absolute address that might need to be adjusted once all
  // the relocations are done. Example: A conditional jump

  // cc is true -> InstructionB otherwise -> InstructionA, InstructionB

  // 0x0100: jcc rip+4 (==0x0104)
  // 0x0102: InstructionA
  // 0x0104: InstructionB
  // ->
  // 0x0200: j(!cc) rip+10 (== 0x0210)
  // 0x0202: jmp [rip+6] (== [0x0208])
  // 0x0208: 8 byte destination address == address of relocated InstructionB == 0x0217
  // 0x0210: InstructionA
  // 0x0217: InstructionB
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
    // We stored the non-relocated address in the instruction above. In case it belongs to an
    // instruction that was relocated this will be taken care of later:
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
    absl::flat_hash_map<uint64_t, uint64_t>& relocation_map, MachineCode& trampoline) {
  cs_insn* instruction = cs_malloc(capstone_handle);
  CHECK(instruction != nullptr);
  orbit_base::unique_resource scope_exit{instruction,
                                         [](cs_insn* instruction) { cs_free(instruction, 1); }};

  std::vector<uint8_t> trampoline_code;
  const uint8_t* code_pointer = function.data();
  size_t code_size = function.size();
  uint64_t disassemble_address = function_address;
  std::vector<size_t> relocateable_addresses;
  while ((disassemble_address - function_address < kSizeOfJmp) &&
         cs_disasm_iter(capstone_handle, &code_pointer, &code_size, &disassemble_address,
                        instruction)) {
    const uint64_t original_instruction_address = disassemble_address - instruction->size;
    const uint64_t relocated_instruction_address =
        trampoline_address + trampoline.GetResultAsVector().size() + trampoline_code.size();
    relocation_map.insert_or_assign(original_instruction_address, relocated_instruction_address);
    // LOG("reloc: %#x -> %#x", original_instruction_address, relocated_instruction_address);
    //  For now just copy - this is the place to instert the non trivial relocation.
    //  This might return an error if the code cannot be relocated.
    auto instruction_or_error = RelocateInstruction(instruction, original_instruction_address,
                                                    relocated_instruction_address);
    if (instruction_or_error.has_error()) {
      return instruction_or_error.error();  // better error message
    }
    if (instruction_or_error.value().position_of_absolute_address.has_value()) {
      const size_t offset = instruction_or_error.value().position_of_absolute_address.value();
      const size_t instruction_address = trampoline_code.size();
      relocateable_addresses.push_back(instruction_address + offset);
    }
    trampoline_code.insert(trampoline_code.end(), instruction_or_error.value().code.begin(),
                           instruction_or_error.value().code.end());
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
  AppendPayloadCode(payload_address, trampoline);
  AppendRestoreCode(trampoline);

  // Relocate prolog into trampoline
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
  AppendPayloadCode(0 /* payload_address*/, unused_code);
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

TEST(DisassemblerTest, Relocate) {
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

  // Examples with modrm & 0xC7 == 0x05
  // mnemonic op_str: add qword ptr [rip + 0x969433], 1
  // modrm: 0x05
  // machine code: 0x48 83 05 33 94 96 00 01

  // mnemonic op_str: lea rbx, [rip + 0x98fab3]
  // modrm: 0x1d
  // machine code: 0x48 8d 1d b3 fa 98 00

  // mnemonic op_str: mov rdi, qword ptr [rip + 0x9649b4]
  // modrm: 0x3d
  //  machine code: 0x48 8b 3d b4 49 96 00

  // mnemonic op_str: lea rdx, [rip + 0x9649bd]
  // modrm: 0x15
  // machine code: 0x48 8d 15 bd 49 96 00

  // mnemonic op_str: movups xmm0, xmmword ptr [rip + 0x63f2b7]
  // modrm: 0x05
  // machine code: 0x0f 10 05 b7 f2 63 00

  // mnemonic op_str: lea rdi, [rip + 0x63f195]
  // modrm: 0x3d
  // machine code: 0x48 8d 3d 95 f1 63 00

  // mnemonic op_str: lea rcx, [rip + 0x92bc2f]
  // modrm: 0x0d
  // machine code: 0x48 8d 0d 2f bc 92 00

  // mnemonic op_str: lea rsi, [rip + 0x63f13a]
  // modrm: 0x35
  // machine code: 0x48 8d 35 3a f1 63 00

  // mnemonic op_str: mov qword ptr [rip + 0x98f8fe], rbx
  // modrm: 0x1d
  // machine code: 0x48 89 1d fe f8 98 00

  // Init Capstone disassembler.
  csh capstone_handle = 0;
  cs_err error_code = cs_open(CS_ARCH_X86, CS_MODE_64, &capstone_handle);
  CHECK(error_code == CS_ERR_OK);
  error_code = cs_option(capstone_handle, CS_OPT_DETAIL, CS_OPT_ON);
  CHECK(error_code == CS_ERR_OK);

  CHECK(AttachAndStopProcess(pid).has_value());

  // const AddressRange address_range_code =
  //     GetFunctionAddressRangeOrDie(pid, "_GLOBAL__sub_I_FindFunctionAddressTest.cpp");
  const AddressRange address_range_code = GetFunctionAddressRangeOrDie(pid, "DoSomething");
  const uint64_t address_of_do_something = address_range_code.start;
  const uint64_t size_of_do_something = address_range_code.end - address_range_code.start;
  LOG("address_of_do_something: %x size_of_do_something: %x", address_of_do_something,
      size_of_do_something);

  // Copy the start of the function over into this process.
  ErrorMessageOr<std::vector<uint8_t>> function_backup =
      ReadTraceesMemory(pid, address_of_do_something, size_of_do_something);
  CHECK(function_backup.has_value());

  {
    cs_insn* instruction = cs_malloc(capstone_handle);
    CHECK(instruction != nullptr);
    orbit_base::unique_resource scope_exit{instruction,
                                           [](cs_insn* instruction) { cs_free(instruction, 1); }};

    const uint8_t* code_pointer = function_backup.value().data();
    size_t code_size = size_of_do_something;
    uint64_t disassemble_address = address_of_do_something;
    while (cs_disasm_iter(capstone_handle, &code_pointer, &code_size, &disassemble_address,
                          instruction)) {
      auto result = RelocateInstruction(instruction, disassemble_address - instruction->size,
                                        disassemble_address - instruction->size + 0x100000);
      CHECK(!result.has_error());
    }
  }

  DumpDissasembly(capstone_handle, function_backup.value(), address_of_do_something);

  cs_close(&capstone_handle);

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

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

  // Copy the start of the function over into this process.
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
  // orbit_base::unique_resource scope_exit{
  //     trampoline_address, [max_trampoline_size, pid](uint64_t trampoline_address) {
  //       auto free_result = FreeInTracee(pid, trampoline_address, max_trampoline_size);
  //       if (free_result.has_error()) {
  //         ERROR("Unable to free memory for trampolines.");
  //       }
  //     }};

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
      0 /* payload_address*/, capstone_handle, relocation_map);

  // Move every instruction pointer that was in the middle of a overwritten function prolog to the
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

  // DEBUG

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

  cs_close(&capstone_handle);

  // Restart the tracee briefly to assert the thing is still running.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  CHECK(AttachAndStopProcess(pid).has_value());

  // Remove the instrumentation (restore the function prologs and deallocate the trampolines)
  // TBD

  // Restart the tracee briefly to assert the thing is still running.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  CHECK(AttachAndStopProcess(pid).has_value());

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);

  // ErrorMessageOr<int> length_or_error =
  //     LengthOfOverriddenInstructions(handle, function_backup.value(), 5);
  // ASSERT_FALSE(length_or_error.has_error());
  // const size_t length_override = length_or_error.value();

  // LOG("length_override: %u", length_override);
  // if (rip > address_of_do_something && rip < address_of_do_something + length_override) {
  //   LOG("********** rip: %#x , address_of_do_something: %#x , length_override: %#x\n", rip,
  //       address_of_do_something, length_override);
  // } else {
  //   // override 5 bytes with jmp, write the modified chunk to tracee and
  //   // keep a copy of the original code in the tracer.
  //   std::vector<uint8_t> backup_prolog(length_override);
  //   memcpy(backup_prolog.data(), function_backup.value().data(), length_override);

  //   // get some memory for the trampoline
  //   auto trampoline_or_error = AllocateMemoryForTrampolines(pid, address_range_code, 1024 *
  //   1024); CHECK(!trampoline_or_error.has_error()); const uint64_t address_trampoline =
  //   trampoline_or_error.value();

  //   // Override beginning of `DoSomething` with a jmp.
  //   MachineCode code;
  //   code.AppendBytes({0xe9}).AppendImmediate32(address_trampoline - (address_of_do_something +
  //   5));
  //   // Not strictly necessary: fill the instructions we override with nop's so we leave no
  //   garbage
  //   // code here.
  //   while (code.GetResultAsVector().size() < length_override) code.AppendBytes({0x90});
  //   auto result_write_code =
  //       WriteTraceesMemory(pid, address_of_do_something, code.GetResultAsVector());
  //   CHECK(!result_write_code.has_error());

  //   // Flush cache so the change becomes active
  //   // CHECK(!FlushCacheLineInTracee(pid, address_of_do_something).has_error());

  //   // disassemble jmp
  //   cs_insn* insn_jmp = nullptr;
  //   size_t count_jmp =
  //       cs_disasm(handle, static_cast<const uint8_t*>(code.GetResultAsVector().data()),
  //                 code.GetResultAsVector().size(), address_of_do_something, 0, &insn_jmp);
  //   size_t k_jmp;
  //   for (k_jmp = 0; k_jmp < count_jmp; k_jmp++) {
  //     LOG("0x%llx:\t%-12s %s", insn_jmp[k_jmp].address, insn_jmp[k_jmp].mnemonic,
  //         insn_jmp[k_jmp].op_str);
  //   }
  //   // Print out the next offset, after the last instruction.
  //   LOG("0x%llx:", insn_jmp[k_jmp - 1].address + insn_jmp[k_jmp - 1].size);

  //   // copy the stuff we overrode into the trampoline - this only works in case the instructions
  //   do
  //   // not use relative addressing.
  //   // backup and restore registers is also missing here.
  //   MachineCode code_trampoline;
  //   std::vector<uint8_t> original_instructions(backup_prolog.begin(),
  //                                              backup_prolog.begin() + length_override);
  //   CHECK(original_instructions.size() == length_override);
  //   code_trampoline.AppendBytes(original_instructions)
  //       .AppendBytes({0xe9})
  //       .AppendImmediate32(address_of_do_something - (address_trampoline + 5));
  //   auto result_write_trampoline =
  //       WriteTraceesMemory(pid, address_trampoline, code_trampoline.GetResultAsVector());
  //   CHECK(!result_write_trampoline.has_error());

  //   LOG("address_of_do_something: %x ", address_of_do_something);
  //   LOG("address_trampoline: %x ", address_trampoline);

  //   // disassemble trampoline
  //   cs_insn* insn_tra = nullptr;
  //   size_t count_tra =
  //       cs_disasm(handle, static_cast<const
  //       uint8_t*>(code_trampoline.GetResultAsVector().data()),
  //                 code_trampoline.GetResultAsVector().size(), address_trampoline, 0, &insn_tra);

  //   size_t k;
  //   for (k = 0; k < count_tra; k++) {
  //     LOG("0x%llx:\t%-12s %s", insn_tra[k].address, insn_tra[k].mnemonic, insn_tra[k].op_str);
  //   }
  //   // Print out the next offset, after the last instruction.
  //   LOG("0x%llx:", insn_tra[k - 1].address + insn_tra[k - 1].size);

  //   LOG("1");
  //   CHECK(!DetachAndContinueProcess(pid).has_error());
  //   LOG("1");
  //   std::this_thread::sleep_for(std::chrono::milliseconds(3));
  //   LOG("1");
  //   CHECK(!AttachAndStopProcess(pid).has_error());
  //   LOG("1");

  //   // write back original function
  //   auto result_write_original_code =
  //       WriteTraceesMemory(pid, address_of_do_something, backup_prolog);

  //   ASSERT_FALSE(FreeInTracee(pid, address_trampoline, 1024 * 1024).has_error());

  //   // Free memory allocated by cs_disasm().
  //   cs_free(insn_jmp, count_jmp);
  //   cs_free(insn_tra, count_tra);
  // }
  // cs_close(&handle);

  // // Detach and end child.
  // CHECK(!DetachAndContinueProcess(pid).has_error());
  // kill(pid, SIGKILL);
  // waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation