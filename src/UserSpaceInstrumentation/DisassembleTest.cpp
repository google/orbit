// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <capstone/capstone.h>
#include <gtest/gtest.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <limits>
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
#include "RegisterState.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_elf_utils::ElfFile;
using orbit_elf_utils::ReadModules;

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
  return i + 42;
}

}  // namespace

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

ErrorMessageOr<std::vector<uint8_t>> RelocateInstruction(cs_insn* instruction) {
  std::vector<uint8_t> result;

  // Instructions using RIP relative addressing. (ModR/M = 00???101B)
  if ((instruction->detail->x86.modrm & 0xC7) == 0x05) {

    // // Modify the RIP relative address.
    // PUINT32 pRelAddr;

    // // Avoid using memcpy to reduce the footprint.
    // memcpy(instBuf, (LPBYTE)pOldInst, copySize);
    // pCopySrc = instBuf;

    // // Relative address is stored at (instruction length - immediate value length - 4).
    // pRelAddr = (PUINT32)(instBuf + hs.len - ((hs.flags & 0x3C) >> 2) - 4);
    // *pRelAddr = (UINT32)((pOldInst + hs.len + (INT32)hs.disp.disp32) - (pNewInst + hs.len));

    // // Complete the function if JMP (FF /4).
    // if (hs.opcode == 0xFF && hs.modrm_reg == 4) finished = TRUE;
  } else {
    result.resize(instruction->size);
    memcpy(result.data(), instruction->bytes, instruction->size);
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

  bool first_instruction = true;
  const uint8_t* code_pointer = function.data();
  size_t code_size = function.size();
  uint64_t disassemble_address = function_address;
  while ((disassemble_address - function_address < kSizeOfJmp) &&
         cs_disasm_iter(capstone_handle, &code_pointer, &code_size, &disassemble_address,
                        instruction)) {
    if (!first_instruction) {
      const uint64_t relocated_instruction_address =
          trampoline_address + trampoline.GetResultAsVector().size();
      relocation_map.insert_or_assign(disassemble_address - instruction->size,
                                      relocated_instruction_address);
      LOG("reloc: %#x -> %#x", disassemble_address - instruction->size,
          relocated_instruction_address);
    } else {
      first_instruction = false;
    }
    // For now just copy - this is the place to instert the non trivial relocation.
    // This might return an error if the code cannot be relocated.
    auto instruction_or_error = RelocateInstruction(instruction);
    if (instruction_or_error.has_error()) {
      return instruction_or_error.error();  // better error message
    }
    trampoline.AppendBytes(instruction_or_error.value());
  }
  address_after_prolog = disassemble_address;
  return outcome::success();
}

void AppendJumpBackCode(uint64_t address_after_prolog, uint64_t trampoline_address,
                        MachineCode& trampoline) {
  const uint64_t address_after_jmp =
      trampoline_address + trampoline.GetResultAsVector().size() + kSizeOfJmp;
  trampoline.AppendBytes({0xe9});
  trampoline.AppendImmediate32(address_after_prolog - address_after_jmp);
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
  AppendJumpBackCode(address_after_prolog, trampoline_address, trampoline);

  // Copy trampoline into tracee.
  auto write_result_or_error =
      WriteTraceesMemory(pid, trampoline_address, trampoline.GetResultAsVector());
  if (write_result_or_error.has_error()) {
    return write_result_or_error.error();
  }

  // Overwrite prolog with jump into the trampoline.
  MachineCode jump;
  jump.AppendBytes({0xe9});
  jump.AppendImmediate32(trampoline_address - (function_address + kSizeOfJmp));
  while (jump.GetResultAsVector().size() < address_after_prolog - function_address) {
    jump.AppendBytes({0x90});
  }
  write_result_or_error = WriteTraceesMemory(pid, function_address, jump.GetResultAsVector());
  if (write_result_or_error.has_error()) {
    return write_result_or_error.error();
  }

  return outcome::success();
}

// ErrorMessageOr<int32_t> AddressDifferenceAsInt32(uint64_t a, uint64_t b) {
//   const uint64_t abs_diff = (a > b) ? (a - b) : (b - a);
//   constexpr uint64_t kAbsMaxInt32AsUint64 =
//       static_cast<uint64_t>(std::numeric_limits<int32_t>::max());
//   constexpr uint64_t kAbsMinInt32AsUint64 =
//       static_cast<uint64_t>(-static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
//   if ((a > b && abs_diff > kAbsMaxInt32AsUint64) ||
//       (b > a && abs_diff > kAbsMinInt32AsUint64)) {
//     return ErrorMessage("Difference is larger than +-2GB.");
//   }
//   return a - b;
// }

// TEST(DisassemblerTest, AddressDifferenceAsInt32) {
//   // Result of the difference is negative; in the first case it just fits the second case
//   overflows. const uint64_t a = 0x6012345612345678; const uint64_t b1 = a -
//   std::numeric_limits<int32_t>::min(); auto result = AddressDifferenceAsInt32(a, b1);
//   ASSERT_FALSE(result.has_error());
//   EXPECT_EQ(std::numeric_limits<int32_t>::min(), result.value());
//   result = AddressDifferenceAsInt32(a, b1 + 1);
//   EXPECT_TRUE(result.has_error());

//   // Result of the difference is positive; in the first case it just fits the second case
//   overflows. const uint64_t b2 = a - std::numeric_limits<int32_t>::max(); result =
//   AddressDifferenceAsInt32(a, b2); ASSERT_FALSE(result.has_error());
//   EXPECT_EQ(std::numeric_limits<int32_t>::max(), result.value());
//   EXPECT_TRUE(AddressDifferenceAsInt32(a, b2 - 1).has_error());

//   // Result of the difference does not even fit into a int64. We handle that gracefully as well.
//   const uint64_t c = 0xa012345612345678;
//   const uint64_t d = c - 0x9012345612345678;
//   result = AddressDifferenceAsInt32(c, d);
//   EXPECT_TRUE(result.has_error());
//   result = AddressDifferenceAsInt32(d, c);
//   EXPECT_TRUE(result.has_error());
// }

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
  AppendJumpBackCode(0 /*address_after_prolog*/, 0 /*trampoline_address*/, unused_code);
  // Round up to the next multiple of eight so we get aligned jump targets at the beginning of the
  // each trampoline.
  return ((unused_code.GetResultAsVector().size() + 7) / 8) * 8;
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