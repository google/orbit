// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/match.h>
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
#include <thread>
#include <vector>

#include "AccessTraceesMemory.h"
#include "AllocateInTracee.h"
#include "ElfUtils/ElfFile.h"
#include "ElfUtils/LinuxMap.h"
#include "MachineCode.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "RegisterState.h"
#include "Trampoline.h"
#include "UserSpaceInstrumentation/Attach.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_elf_utils::ElfFile;
using orbit_elf_utils::ReadModules;

uint64_t g_a = 0;

extern "C" __attribute__((noinline)) int DoSomething(int i) {
  g_a++;
  return i + 42;
}
// void FreeMemoryOrDie(pid_t pid, uint64_t address_code, uint64_t size) {
//   auto result = FreeInTracee(pid, address_code, size);
//   FAIL_IF(result.has_error(), "Unable to free previously allocated memory in tracee: \"%s\"",
//           result.error().message());
// }
// void RestoreRegistersOrDie(RegisterState& register_state) {
//   auto result = register_state.RestoreRegisters();
//   FAIL_IF(result.has_error(), "Unable to restore register state in tracee: \"%s\"",
//           result.error().message());
// }

}  // namespace

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

// ErrorMessageOr<void> FlushCacheLineInTracee(pid_t pid, uint64_t address) {
//   MachineCode code;
//   code.AppendBytes({0x0f, 0xae}).AppendImmediate64(address).AppendBytes({0xcc});
//   // Allocate small memory area in the tracee. This is used for the code and the symbol name.
//   const uint64_t memory_size = code.GetResultAsVector().size();
//   OUTCOME_TRY(address_code, AllocateInTracee(pid, 0, memory_size));

//   auto result_write_code = WriteTraceesMemory(pid, address_code, code.GetResultAsVector());
//   if (result_write_code.has_error()) {
//     //FreeMemoryOrDie(pid, address_code, memory_size);
//     return result_write_code.error();
//   }

//   // Backup registers.
//   RegisterState original_registers;
//   OUTCOME_TRY(original_registers.BackupRegisters(pid));

//   RegisterState registers_set_rip = original_registers;
//   registers_set_rip.GetGeneralPurposeRegisters()->x86_64.rip = address_code;
//   OUTCOME_TRY(registers_set_rip.RestoreRegisters());
//   if (ptrace(PTRACE_CONT, pid, 0, 0) != 0) {
//     FATAL("Unable to continue tracee with PTRACE_CONT.");
//   }
//   int status = 0;
//   pid_t waited = waitpid(pid, &status, 0);
//   if (waited != pid || !WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP) {
//     FATAL(
//         "Failed to wait for sigtrap after PTRACE_CONT. Expected pid: %d Pid returned from
//         waitpid: "
//         "%d status: %u, WIFSTOPPED: %u, WSTOPSIG: %u",
//         pid, waited, status, WIFSTOPPED(status), WSTOPSIG(status));
//   }

//   // Clean up memory and registers.
//   RestoreRegistersOrDie(original_registers);
//   FreeMemoryOrDie(pid, address_code, memory_size);
//   return outcome::success();
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

  // Stop the child process using our tooling.
  CHECK(AttachAndStopProcess(pid).has_value());

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
  uint64_t address_do_something = 0;
  uint64_t size_do_something = 0;
  for (const auto& sym : syms.value().symbol_infos()) {
    if (sym.name() == "DoSomething") {
      address_do_something = sym.address() + address_range_code.start - syms.value().load_bias();
      size_do_something = sym.size();
    }
  }

  // Backup registers.
  RegisterState original_registers;
  CHECK(!original_registers.BackupRegisters(pid).has_error());
  const uint64_t rip = original_registers.GetGeneralPurposeRegisters()->x86_64.rip;
  LOG("rip: %#x", rip);

  // Copy the entire function over into this process.
  auto function_backup = ReadTraceesMemory(pid, address_do_something, size_do_something);
  CHECK(function_backup.has_value());

  // Disassemble the function.
  csh handle = 0;
  cs_insn* insn = nullptr;
  cs_err err = cs_open(CS_ARCH_X86, CS_MODE_64, &handle);
  CHECK(err == 0);
  size_t count = cs_disasm(handle, static_cast<const uint8_t*>(function_backup.value().data()),
                           size_do_something, address_do_something, 0, &insn);

  size_t j;
  for (j = 0; j < count; j++) {
    LOG("0x%llx:\t%-12s %s", insn[j].address, insn[j].mnemonic, insn[j].op_str);
  }
  // Print out the next offset, after the last instruction.
  LOG("0x%llx:", insn[j - 1].address + insn[j - 1].size);

  size_t length_override = 0;
  for (size_t i = 0; i < count; i++) {
    length_override += insn[i].size;
    if (length_override >= 5) break;
  }
  CHECK(length_override >= 5);  // function too short?!
  LOG("length_override: %u", length_override);
  if (rip >= address_do_something && rip < address_do_something + length_override) {
    LOG("********** rip: %#x , address_do_something: %#x , length_override: %#x\n", rip,
        address_do_something, length_override);
  }

  // override 5 bytes with jmp, write the modified chunk to tracee and
  // keep a copy of the original code in the tracer.
  std::vector<uint8_t> tmp_prolog(length_override);
  memcpy(tmp_prolog.data(), function_backup.value().data(), length_override);
  std::vector<uint8_t> backup_prolog(tmp_prolog);

  // get some memory for the trampoline
  auto trampoline_or_error = AllocateMemoryForTrampolines(pid, address_range_code, 1024 * 1024);
  CHECK(!trampoline_or_error.has_error());
  const uint64_t address_trampoline = trampoline_or_error.value();

  // Override beginning of `DoSomething` with a jmp.
  MachineCode code;
  code.AppendBytes({0xe9}).AppendImmediate32(address_trampoline - (address_do_something + 5));
  // Not strictly necessary: fill the instructions we override with nop's so we leave no garbage
  // code here.
  while (code.GetResultAsVector().size() < length_override) code.AppendBytes({0x90});
  for (int i = 0; i < 5; i++) tmp_prolog[i] = code.GetResultAsVector()[i];
  auto result_write_code = WriteTraceesMemory(pid, address_do_something, tmp_prolog);
  CHECK(!result_write_code.has_error());

  // Flush cache so the change becomes active
  // CHECK(!FlushCacheLineInTracee(pid, address_do_something).has_error());

  // disassemble jmp
  cs_insn* insn_jmp = nullptr;
  size_t count_jmp = cs_disasm(handle, static_cast<const uint8_t*>(code.GetResultAsVector().data()),
                               code.GetResultAsVector().size(), address_do_something, 0, &insn_jmp);
  size_t k_jmp;
  for (k_jmp = 0; k_jmp < count_jmp; k_jmp++) {
    LOG("0x%llx:\t%-12s %s", insn_jmp[k_jmp].address, insn_jmp[k_jmp].mnemonic,
        insn_jmp[k_jmp].op_str);
  }
  // Print out the next offset, after the last instruction.
  LOG("0x%llx:", insn_jmp[k_jmp - 1].address + insn_jmp[k_jmp - 1].size);

  // copy the stuff we overrode into the trampoline - this only works in case the instructions do
  // not use relative addressing.
  // backup and restore registers is also missing here.
  MachineCode code_trampoline;
  std::vector<uint8_t> original_instructions(backup_prolog.begin(),
                                             backup_prolog.begin() + length_override);
  CHECK(original_instructions.size() == length_override);
  code_trampoline.AppendBytes(original_instructions)
      .AppendBytes({0xe9})
      .AppendImmediate32(address_do_something - (address_trampoline + 5));
  auto result_write_trampoline =
      WriteTraceesMemory(pid, address_trampoline, code_trampoline.GetResultAsVector());
  CHECK(!result_write_trampoline.has_error());

  LOG("address_do_something: %x ", address_do_something);
  LOG("address_trampoline: %x ", address_trampoline);

  // disassemble trampoline
  cs_insn* insn_tra = nullptr;
  size_t count_tra =
      cs_disasm(handle, static_cast<const uint8_t*>(code_trampoline.GetResultAsVector().data()),
                code_trampoline.GetResultAsVector().size(), address_trampoline, 0, &insn_tra);

  size_t k;
  for (k = 0; k < count_tra; k++) {
    LOG("0x%llx:\t%-12s %s", insn_tra[k].address, insn_tra[k].mnemonic, insn_tra[k].op_str);
  }
  // Print out the next offset, after the last instruction.
  LOG("0x%llx:", insn_tra[k - 1].address + insn_tra[k - 1].size);

  LOG("1");
  CHECK(!DetachAndContinueProcess(pid).has_error());
  LOG("1");
  std::this_thread::sleep_for(std::chrono::milliseconds(3));
  LOG("1");
  CHECK(!AttachAndStopProcess(pid).has_error());
  LOG("1");

  // write back original function
  auto result_write_original_code = WriteTraceesMemory(pid, address_do_something, backup_prolog);

  ASSERT_FALSE(FreeInTracee(pid, address_trampoline, 1024 * 1024).has_error());

  // Free memory allocated by cs_disasm().
  cs_free(insn, count);
  cs_free(insn_jmp, count_jmp);
  cs_free(insn_tra, count_tra);
  cs_close(&handle);

  // Detach and end child.
  CHECK(!DetachAndContinueProcess(pid).has_error());
  kill(pid, SIGKILL);
  waitpid(pid, NULL, 0);
}

}  // namespace orbit_user_space_instrumentation