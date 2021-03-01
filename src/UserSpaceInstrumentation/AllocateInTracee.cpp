// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AllocateInTracee.h"

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

#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/SafeStrerror.h"
#include "UserSpaceInstrumentation/RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

using orbit_base::ReadFileToString;

// Returns the address range of the first executable memory region. In every case I encountered this
// was the second line in the `maps` file corresponding to the code of the process we look at.
// However we don't really care. So keeping it general and just searching for an executable region
// is probably helping stability.
[[nodiscard]] ErrorMessageOr<void> GetFirstExecutableMemoryRegion(pid_t pid, uint64_t* addr_start,
                                                                  uint64_t* addr_end) {
  auto result_read_maps = ReadFileToString(absl::StrFormat("/proc/%d/maps", pid));
  if (result_read_maps.has_error()) {
    return result_read_maps.error();
  }
  const std::vector<std::string> lines =
      absl::StrSplit(result_read_maps.value(), '\n', absl::SkipEmpty());
  for (const auto& line : lines) {
    const std::vector<std::string> tokens = absl::StrSplit(line, ' ', absl::SkipEmpty());
    if (tokens.size() < 2 || tokens[1].size() != 4 || tokens[1][2] != 'x') continue;
    const std::vector<std::string> addresses = absl::StrSplit(tokens[0], '-');
    if (addresses.size() != 2) continue;
    *addr_start = std::stoull(addresses[0], nullptr, 16);
    *addr_end = std::stoull(addresses[1], nullptr, 16);
    return outcome::success();
  }
  return ErrorMessage(absl::StrFormat("Unable to locate executable memory area in pid: %d", pid));
}

// Write `bytes` into memory of process `pid` starting from `address_start`.
// Note that we write multiples of eight bytes at once. If length of `bytes` is not a multiple of
// eight the remaining bytes are zeroed out.
void WriteBytesIntoTraceesMemory(pid_t pid, uint64_t address_start,
                                 const std::vector<uint8_t>& bytes) {
  size_t pos = 0;
  do {
    // Pack 8 byte for writing into `data`.
    uint64_t data = 0;
    for (size_t i = 0; i < sizeof(data) && pos + i < bytes.size(); i++) {
      uint64_t t = bytes[pos + i];
      data |= t << (8 * i);
    }
    CHECK(ptrace(PTRACE_POKEDATA, pid, address_start + pos, data) != -1);
    pos += 8;
  } while (pos < bytes.size());
}

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
  uint64_t address_start = 0;
  uint64_t address_end = 0;
  auto result_memory_region = GetFirstExecutableMemoryRegion(pid, &address_start, &address_end);
  if (result_memory_region.has_error()) {
    return ErrorMessage(absl::StrFormat("Failed to find executable memory region: \"%s\"",
                                        result_memory_region.error().message()));
  }

  // Backup first 8 bytes.
  const uint64_t backup_module_code = ptrace(PTRACE_PEEKDATA, pid, address_start, NULL);
  if (errno) {
    return ErrorMessage(absl::StrFormat("Failed to PTRACE_PEEKDATA with errno %d: \"%s\"", errno,
                                        SafeStrerror(errno)));
  }

  // Write `syscall` into memory. Machine code is `0x0f05`.
  WriteBytesIntoTraceesMemory(pid, address_start, std::vector<uint8_t>{0x0f, 0x05});

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
  // Syscalls return -4095, ..., -1 on failure.
  const int64_t result_as_int = reinterpret_cast<uint64_t>(result);
  if (result_as_int > -4096 && result_as_int < 0) {
    return ErrorMessage(absl::StrFormat("syscall failed. Return value: %u", result));
  }

  // Clean up memory and registers.
  auto result_restore_memory = ptrace(PTRACE_POKEDATA, pid, address_start, backup_module_code);
  if (result_restore_memory == -1) {
    FATAL("Unable to restore memory state of tracee");
  }
  result_restore_registers = original_registers.RestoreRegisters();
  if (result_restore_registers.has_error()) {
    FATAL("Unable to restore register state of tracee : \"%s\"",
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
    ErrorMessage(absl::StrFormat("Failed to execute mmap syscall: \"%s\"",
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
    ErrorMessage(absl::StrFormat("Failed to execute munmap syscall: \"%s\"",
                                 result_or_error.error().message()));
  }
  return outcome::success();
}

}  // namespace orbit_user_space_instrumentation