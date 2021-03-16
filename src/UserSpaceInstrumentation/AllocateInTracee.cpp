// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AllocateInTracee.h"

#include <absl/base/casts.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <cstdint>
#include <string>
#include <vector>

#include "AccessTraceesMemory.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

// Execute a single syscall instruction in tracee `pid`. `syscall` identifies the syscall as in this
// list: https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_64.tbl
// The parameters and the ordering are the same as in the C wrapper:
// https://man7.org/linux/man-pages/dir_section_2.html
[[nodiscard]] ErrorMessageOr<uint64_t> SyscallInTracee(pid_t pid, uint64_t syscall, uint64_t arg_0,
                                                       uint64_t arg_1 = 0, uint64_t arg_2 = 0,
                                                       uint64_t arg_3 = 0, uint64_t arg_4 = 0,
                                                       uint64_t arg_5 = 0) {
  RegisterState original_registers;
  auto result_backup = original_registers.BackupRegisters(pid);
  if (result_backup.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to backup original register state: \"%s\"",
                                        result_backup.error().message()));
  }
  if (original_registers.GetBitness() != RegisterState::Bitness::k64Bit) {
    return ErrorMessage(
        "Tried to invoke syscall in 32 bit process. This is currently not supported.");
  }

  // Get an executable memory region.
  auto result_memory_region = GetFirstExecutableMemoryRegion(pid);
  if (result_memory_region.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to find executable memory region: \"%s\"",
                                        result_memory_region.error().message()));
  }
  const uint64_t address_start = result_memory_region.value().first;

  // Backup first 8 bytes.
  auto result_backup_code = ReadTraceesMemory(pid, address_start, 8);
  if (result_backup_code.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to read from tracee's memory: \"%s\"",
                                        result_backup_code.error().message()));
  }

  // Write `syscall` into memory. Machine code is `0x0f05`.
  auto result_write_code = WriteTraceesMemory(pid, address_start, std::vector<uint8_t>{0x0f, 0x05});
  if (result_write_code.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to write to tracee's memory: \"%s\"",
                                        result_write_code.error().message()));
  }

  // Move instruction pointer to the `syscall` and fill registers with parameters.
  RegisterState registers_for_syscall = original_registers;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rip = address_start;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rax = syscall;
  // Register list for arguments can be found e.g. in the glibc wrapper:
  // https://github.com/bminor/glibc/blob/master/sysdeps/unix/sysv/linux/x86_64/syscall.S#L30
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rdi = arg_0;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rsi = arg_1;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rdx = arg_2;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.r10 = arg_3;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.r8 = arg_4;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.r9 = arg_5;
  auto result_restore_registers = registers_for_syscall.RestoreRegisters();
  if (result_restore_registers.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to set registers with syscall parameters: \"%s\"",
                                        result_restore_registers.error().message()));
  }

  // Single step to execute the syscall.
  if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) != 0) {
    return ErrorMessage("Failed to execute syscall with PTRACE_SINGLESTEP.");
  }
  int status = 0;
  pid_t waited = waitpid(pid, &status, 0);
  if (waited != pid || !WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP) {
    return ErrorMessage("Failed to wait for PTRACE_SINGLESTEP to execute.");
  }

  // Return value of syscalls is in rax.
  RegisterState return_value;
  auto result_backup_return_value = return_value.BackupRegisters(pid);
  if (result_backup_return_value.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to get registers with mmap result: \"%s\"",
                                        result_backup_return_value.error().message()));
  }
  const uint64_t result = return_value.GetGeneralPurposeRegisters()->x86_64.rax;
  // Syscalls return -4095, ..., -1 on failure. And these are actually (-1 * errno)
  const int64_t result_as_int = absl::bit_cast<uint64_t>(result);
  if (result_as_int > -4096 && result_as_int < 0) {
    return ErrorMessage(absl::StrFormat("syscall failed. Return value: %s (%d)",
                                        SafeStrerror(-result_as_int), result_as_int));
  }

  // Clean up memory and registers.
  auto result_restore_memory = WriteTraceesMemory(pid, address_start, result_backup_code.value());
  if (result_restore_memory.has_error()) {
    FATAL("Unable to restore memory state of tracee: \"%s\"",
          result_restore_memory.error().message());
  }
  result_restore_registers = original_registers.RestoreRegisters();
  if (result_restore_registers.has_error()) {
    FATAL("Unable to restore register state of tracee: \"%s\"",
          result_restore_registers.error().message());
  }

  return result;
}

}  // namespace

[[nodiscard]] ErrorMessageOr<uint64_t> AllocateInTracee(pid_t pid, uint64_t size) {
  // Syscall will be equivalent to:
  // `mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)`
  constexpr uint64_t kSyscallNumberMmap = 9;
  auto result_or_error = SyscallInTracee(pid, kSyscallNumberMmap, static_cast<uint64_t>(NULL), size,
                                         PROT_READ | PROT_WRITE | PROT_EXEC,
                                         MAP_PRIVATE | MAP_ANONYMOUS, static_cast<uint64_t>(-1), 0);
  if (result_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to execute mmap syscall: \"%s\"",
                                        result_or_error.error().message()));
  }
  return result_or_error.value();
}

[[nodiscard]] ErrorMessageOr<void> FreeInTracee(pid_t pid, uint64_t address, uint64_t size) {
  // Syscall will be equivalent to:
  // `munmap(address, size)`
  constexpr uint64_t kSyscallNumberMunmap = 11;
  auto result_or_error = SyscallInTracee(pid, kSyscallNumberMunmap, address, size);
  if (result_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to execute munmap syscall: \"%s\"",
                                        result_or_error.error().message()));
  }
  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation