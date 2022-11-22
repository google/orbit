// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ExecuteMachineCode.h"

#include <signal.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "AccessTraceesMemory.h"
#include "OrbitBase/Logging.h"
#include "RegisterState.h"

namespace orbit_user_space_instrumentation {

namespace {

// In certain error conditions the tracee is damaged and we don't try to recover from that. We just
// abort with a fatal log message. None of these errors are expected to occur in operation
// obviously. That's what the *OrDie methods below are for.
void RestoreRegistersOrDie(RegisterState& register_state) {
  auto result = register_state.RestoreRegisters();
  ORBIT_FAIL_IF(result.has_error(), "Unable to restore register state in tracee: %s",
                result.error().message());
}

uint64_t GetReturnValueOrDie(pid_t pid) {
  RegisterState return_value_registers;
  auto result_return_value = return_value_registers.BackupRegisters(pid);
  ORBIT_FAIL_IF(result_return_value.has_error(),
                "Unable to read registers after function called: %s",
                result_return_value.error().message());
  return return_value_registers.GetGeneralPurposeRegisters()->x86_64.rax;
}

}  // namespace

ErrorMessageOr<uint64_t> ExecuteMachineCode(MemoryInTracee& code_memory, const MachineCode& code) {
  const pid_t pid = code_memory.GetPid();

  OUTCOME_TRY(WriteTraceesMemory(pid, code_memory.GetAddress(), code.GetResultAsVector()));

  OUTCOME_TRY(code_memory.EnsureMemoryExecutable());

  // Backup registers.
  RegisterState original_registers;
  OUTCOME_TRY(original_registers.BackupRegisters(pid));

  RegisterState registers_for_execution = original_registers;
  registers_for_execution.GetGeneralPurposeRegisters()->x86_64.rip = code_memory.GetAddress();
  const uint64_t old_rsp = original_registers.GetGeneralPurposeRegisters()->x86_64.rsp;
  // The System V calling convention (Linux x64) assumes the 128 bytes below rsp to be usable as a
  // scratch pad for the current function. This area is called the 'red zone'. The function we
  // interrupted might have stored temporary data in the red zone. To keep the frame of the function
  // we are about to execute separate, we decrement rsp by 128 bytes.
  constexpr int kRedZoneSize = 128;
  // We might also be executing a function with Microsoft x64 calling convention (e.g., under Wine).
  // There is no red zone in this calling convention, however we have the "shadow space", 32 bytes
  // *before* the 8 bytes of the return address that are owned by the current function and can also
  // be used as scratch space. This must not intersect with the red zone of the function we
  // interrupted.
  constexpr int kShadowSpaceSize = 32;
  // The calling conventions on both Linux and Windows also ask for rsp to be aligned to 16 bytes,
  // so we additionally round down to the previous multiple of 16.
  constexpr int kStackAlignment = 16;
  const uint64_t aligned_rsp_below_red_zone_and_shadow_space =
      ((old_rsp - kRedZoneSize - kShadowSpaceSize) / kStackAlignment) * kStackAlignment;
  registers_for_execution.GetGeneralPurposeRegisters()->x86_64.rsp =
      aligned_rsp_below_red_zone_and_shadow_space;
  // In case we stopped the process in the middle of a syscall orig_rax holds the number of that
  // syscall. The kernel uses that to trigger the restart of the interrupted syscall. By setting
  // orig_rax to -1 we bypass this logic for the PTRACE_CONT below.
  // The syscall will be restarted when we restore the original registers and detach to continue the
  // normal operation.
  registers_for_execution.GetGeneralPurposeRegisters()->x86_64.orig_rax = -1;
  OUTCOME_TRY(registers_for_execution.RestoreRegisters());
  if (ptrace(PTRACE_CONT, pid, 0, 0) != 0) {
    ORBIT_FATAL("Unable to continue tracee with PTRACE_CONT.");
  }
  int status = 0;
  pid_t waited = waitpid(pid, &status, 0);
  if (waited != pid || !WIFSTOPPED(status) || WSTOPSIG(status) != SIGTRAP) {
    ORBIT_FATAL(
        "Failed to wait for sigtrap after PTRACE_CONT. Expected pid: %d Pid returned from waitpid: "
        "%d status: %u, WIFSTOPPED: %u, WSTOPSIG: %u",
        pid, waited, status, WIFSTOPPED(status), WSTOPSIG(status));
  }

  const uint64_t return_value = GetReturnValueOrDie(pid);

  // Clean up registers.
  RestoreRegistersOrDie(original_registers);
  return return_value;
}

}  // namespace orbit_user_space_instrumentation