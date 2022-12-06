// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AllocateInTracee.h"

#include <absl/base/casts.h>
#include <absl/strings/str_format.h>
#include <linux/seccomp.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/wait.h>

#include <csignal>
#include <cstdint>
#include <cstdlib>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "AccessTraceesMemory.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "ReadSeccompModeOfThread.h"
#include "RegisterState.h"
#include "UserSpaceInstrumentation/AddressRange.h"

namespace orbit_user_space_instrumentation {

namespace {

// Execute a single syscall instruction in tracee `pid`. `syscall` identifies the syscall as in this
// list: https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_64.tbl
// The parameters and the ordering are the same as in the C wrapper:
// https://man7.org/linux/man-pages/dir_section_2.html
// Optionally one can specify `exclude_address`. This prevents the method from using an address
// range containing `exclude_address` as a working area. This is required for the munmap syscall
// which might otherwise choose the mapping it is removing as a working area.
[[nodiscard]] ErrorMessageOr<uint64_t> SyscallInTracee(pid_t pid, uint64_t syscall, uint64_t arg_0,
                                                       uint64_t arg_1, uint64_t arg_2,
                                                       uint64_t arg_3, uint64_t arg_4,
                                                       uint64_t arg_5, uint64_t exclude_address) {
  RegisterState original_registers;
  auto register_backup_result = original_registers.BackupRegisters(pid);
  if (register_backup_result.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to backup original register state: %s",
                                        register_backup_result.error().message()));
  }
  if (original_registers.GetBitness() != RegisterState::Bitness::k64Bit) {
    return ErrorMessage(
        "Tried to invoke syscall in 32 bit process. This is currently not supported.");
  }

  // Get an executable memory region.
  auto memory_region_or_error = GetExistingExecutableMemoryRegion(pid, exclude_address);
  if (memory_region_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to find executable memory region: %s",
                                        memory_region_or_error.error().message()));
  }
  const uint64_t start_address = memory_region_or_error.value().start;

  // Backup first 8 bytes.
  auto backup_or_error = ReadTraceesMemory(pid, start_address, 8);
  if (backup_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to read from tracee's memory: %s",
                                        backup_or_error.error().message()));
  }
  const std::vector<uint8_t> backup = std::move(backup_or_error.value());

  // Write `syscall` into memory. Machine code is `0x0f05`.
  auto write_code_result = WriteTraceesMemory(pid, start_address, std::vector<uint8_t>{0x0f, 0x05});
  if (write_code_result.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to write to tracee's memory: %s",
                                        write_code_result.error().message()));
  }

  std::shared_ptr<void> restore_memory_on_return{
      nullptr, [pid, start_address, backup](void* /*ptr*/) {
        auto restore_memory_result = WriteTraceesMemory(pid, start_address, backup);
        if (restore_memory_result.has_error()) {
          ORBIT_ERROR("Unable to restore memory state of tracee: %s",
                      restore_memory_result.error().message());
        }
      }};

  // Move instruction pointer to the `syscall` and fill registers with parameters.
  RegisterState registers_for_syscall = original_registers;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rip = start_address;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rax = syscall;
  // Register list for arguments can be found e.g. in the glibc wrapper:
  // https://github.com/bminor/glibc/blob/master/sysdeps/unix/sysv/linux/x86_64/syscall.S#L30
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rdi = arg_0;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rsi = arg_1;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.rdx = arg_2;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.r10 = arg_3;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.r8 = arg_4;
  registers_for_syscall.GetGeneralPurposeRegisters()->x86_64.r9 = arg_5;
  auto restore_registers_result = registers_for_syscall.RestoreRegisters();
  if (restore_registers_result.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to set registers with syscall parameters: %s",
                                        restore_registers_result.error().message()));
  }

  std::shared_ptr<void> restore_registers_on_return{
      nullptr, [&original_registers](void* /*ptr*/) {
        auto restore_registers_result = original_registers.RestoreRegisters();
        if (restore_registers_result.has_error()) {
          ORBIT_ERROR("Unable to restore register state of tracee: %s",
                      restore_registers_result.error().message());
        }
      }};

  // The system call could cause the thread to be killed, so we need to read the seccomp mode
  // before actually executing the system call.
  const std::optional<int> seccomp_mode = ReadSeccompModeOfThread(pid);
  std::string seccomp_message_suffix;
  if (seccomp_mode.has_value() && seccomp_mode.value() == SECCOMP_MODE_STRICT) {
    seccomp_message_suffix = absl::StrFormat(
        " This might be due to thread %d being in seccomp mode %d (SECCOMP_MODE_STRICT).", pid,
        SECCOMP_MODE_STRICT);
  } else if (seccomp_mode.has_value() && seccomp_mode.value() == SECCOMP_MODE_FILTER) {
    seccomp_message_suffix = absl::StrFormat(
        " This might be due to thread %d being in seccomp mode %d (SECCOMP_MODE_FILTER).", pid,
        SECCOMP_MODE_FILTER);
  }

  // Single step to execute the syscall.
  if (ptrace(PTRACE_SINGLESTEP, pid, 0, 0) != 0) {
    return ErrorMessage("Failed to execute syscall with PTRACE_SINGLESTEP.");
  }
  int status = 0;
  pid_t waited = waitpid(pid, &status, 0);
  if (waited != pid || !WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP) {
    return ErrorMessage(absl::StrFormat("Failed to wait for PTRACE_SINGLESTEP to execute.%s",
                                        seccomp_message_suffix));
  }

  // Return value of syscalls is in rax.
  RegisterState return_value;
  auto return_value_result = return_value.BackupRegisters(pid);
  if (return_value_result.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to get registers with result of syscall: %s",
                                        return_value_result.error().message()));
  }
  const uint64_t result = return_value.GetGeneralPurposeRegisters()->x86_64.rax;
  // Syscalls return -4095, ..., -1 on failure. And these are actually (-1 * errno)
  const auto result_as_int = absl::bit_cast<int64_t>(result);
  if (result_as_int > -4096 && result_as_int < 0) {
    return ErrorMessage(absl::StrFormat("Syscall failed. Return value: %s (%d).%s",
                                        SafeStrerror(-result_as_int), result_as_int,
                                        seccomp_message_suffix));
  }

  return result;
}

}  // namespace

ErrorMessageOr<std::unique_ptr<MemoryInTracee>> MemoryInTracee::Create(pid_t pid, uint64_t address,
                                                                       uint64_t size) {
  // Syscall will be equivalent to:
  // `mmap(address, size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)`
  // We just set `PROT_WRITE` but this permits also read access (on x86) although the read flag will
  // not show up in /proc/pid/maps. Setting `PROT_READ` explicitly would be clearer but under some
  // circumstances (personality setting READ_IMPLIES_EXEC) `PROT_READ` sets the flag permitting
  // execution and we want to avoid that.
  constexpr uint64_t kSyscallNumberMmap = 9;
  auto result_or_error = SyscallInTracee(pid, kSyscallNumberMmap, address, size, PROT_WRITE,
                                         MAP_PRIVATE | MAP_ANONYMOUS, static_cast<uint64_t>(-1), 0,
                                         /*exclude_address=*/0);
  if (result_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat(
        "Failed to execute mmap syscall with parameters address=%#x size=%u prot=PROT_WRITE: %s",
        address, size, result_or_error.error().message()));
  }

  std::unique_ptr<MemoryInTracee> result(
      new MemoryInTracee(pid, result_or_error.value(), size, MemoryInTracee::State::kWritable));

  if (address != 0 && result->GetAddress() != address) {
    auto free_memory_result = result->Free();
    ORBIT_FAIL_IF(free_memory_result.has_error(), "Unable to free previously allocated memory: %s",
                  free_memory_result.error().message());
    return ErrorMessage(
        absl::StrFormat("MemoryInTracee wanted to allocate memory at %#x but got memory at a "
                        "different address: %#x. The memory has been freed again.",
                        address, result->GetAddress()));
  }

  return result;
}

ErrorMessageOr<void> MemoryInTracee::Free() {
  // Syscall will be equivalent to:
  // `munmap(address, size)`
  constexpr uint64_t kSyscallNumberMunmap = 11;
  auto result_or_error =
      SyscallInTracee(pid_, kSyscallNumberMunmap, address_, size_, 0, 0, 0, 0, address_);
  if (result_or_error.has_error()) {
    return ErrorMessage(
        absl::StrFormat("Failed to execute munmap syscall: %s", result_or_error.error().message()));
  }
  pid_ = -1;
  address_ = 0;
  size_ = 0;
  state_ = State::kWritable;
  return outcome::success();
}

ErrorMessageOr<void> MemoryInTracee::EnsureMemoryExecutable() {
  if (state_ == State::kExecutable) {
    return outcome::success();
  }

  constexpr uint64_t kSyscallNumberMprotect = 10;
  auto result_or_error = SyscallInTracee(pid_, kSyscallNumberMprotect, address_, size_,
                                         PROT_EXEC | PROT_READ, 0, 0, 0, 0);
  if (result_or_error.has_error()) {
    return ErrorMessage(absl::StrFormat(
        "Failed to execute mprotect syscall with parameters address=%#x size=%u prot=PROT_EXEC: %s",
        address_, size_, result_or_error.error().message()));
  }

  state_ = State::kExecutable;
  return outcome::success();
}

ErrorMessageOr<void> MemoryInTracee::EnsureMemoryWritable() {
  if (state_ == State::kWritable) {
    return outcome::success();
  }

  constexpr uint64_t kSyscallNumberMprotect = 10;
  auto result_or_error =
      SyscallInTracee(pid_, kSyscallNumberMprotect, address_, size_, PROT_WRITE, 0, 0, 0, 0);
  if (result_or_error.has_error()) {
    return ErrorMessage(
        absl::StrFormat("Failed to execute mprotect syscall with parameters address=%#x size=%u "
                        "prot=PROT_WRITE: %s",
                        address_, size_, result_or_error.error().message()));
  }

  state_ = State::kWritable;
  return outcome::success();
}

ErrorMessageOr<std::unique_ptr<AutomaticMemoryInTracee>> AutomaticMemoryInTracee::Create(
    pid_t pid, uint64_t address, uint64_t size) {
  // Syscall will be equivalent to:
  // `mmap(address, size, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)`
  // We just set `PROT_WRITE` but this permits also read access (on x86) although the read flag will
  // not show up in /proc/pid/maps. Setting `PROT_READ` explicitly would be clearer but under some
  // circumstances (personality setting READ_IMPLIES_EXEC) `PROT_READ` sets the flag permitting
  // execution and we want to avoid that.
  constexpr uint64_t kSyscallNumberMmap = 9;
  auto result_or_error = SyscallInTracee(pid, kSyscallNumberMmap, address, size, PROT_WRITE,
                                         MAP_PRIVATE | MAP_ANONYMOUS, static_cast<uint64_t>(-1), 0,
                                         /*exclude_address=*/0);
  if (result_or_error.has_error()) {
    return ErrorMessage(
        absl::StrFormat("Failed to execute mmap syscall with parameters address=%#x size=%u: %s",
                        address, size, result_or_error.error().message()));
  }

  std::unique_ptr<AutomaticMemoryInTracee> result(new AutomaticMemoryInTracee(
      pid, result_or_error.value(), size, AutomaticMemoryInTracee::State::kWritable));

  if (address != 0 && result->GetAddress() != address) {
    auto free_memory_result = result->Free();
    ORBIT_FAIL_IF(free_memory_result.has_error(), "Unable to free previously allocated memory: %s",
                  free_memory_result.error().message());
    return ErrorMessage(absl::StrFormat(
        "AutomaticMemoryInTracee wanted to allocate memory at %#x but got memory at a "
        "different address: %#x. The memory has been freed again.",
        address, result->GetAddress()));
  }

  return result;
}

AutomaticMemoryInTracee::~AutomaticMemoryInTracee() {
  if (pid_ == -1) return;  // Freed manually already.
  auto result = Free();
  if (result.has_error()) {
    ORBIT_ERROR("Unable to free memory in tracee: %s", result.error().message());
  }
}

}  // namespace orbit_user_space_instrumentation