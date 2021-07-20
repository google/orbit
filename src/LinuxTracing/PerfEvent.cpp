// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PerfEvent.h"

#include "PerfEventVisitor.h"

namespace orbit_linux_tracing {

std::array<uint64_t, PERF_REG_X86_64_MAX> perf_event_sample_regs_user_all_to_register_array(
    const perf_event_sample_regs_user_all& regs) {
  std::array<uint64_t, PERF_REG_X86_64_MAX> registers{};
  registers[PERF_REG_X86_AX] = regs.ax;
  registers[PERF_REG_X86_BX] = regs.bx;
  registers[PERF_REG_X86_CX] = regs.cx;
  registers[PERF_REG_X86_DX] = regs.dx;
  registers[PERF_REG_X86_SI] = regs.si;
  registers[PERF_REG_X86_DI] = regs.di;
  registers[PERF_REG_X86_BP] = regs.bp;
  registers[PERF_REG_X86_SP] = regs.sp;
  registers[PERF_REG_X86_IP] = regs.ip;
  registers[PERF_REG_X86_FLAGS] = regs.flags;
  registers[PERF_REG_X86_CS] = regs.cs;
  registers[PERF_REG_X86_SS] = regs.ss;
  // Registers ds, es, fs, gs do not actually exist.
  registers[PERF_REG_X86_DS] = 0ul;
  registers[PERF_REG_X86_ES] = 0ul;
  registers[PERF_REG_X86_FS] = 0ul;
  registers[PERF_REG_X86_GS] = 0ul;
  registers[PERF_REG_X86_R8] = regs.r8;
  registers[PERF_REG_X86_R9] = regs.r9;
  registers[PERF_REG_X86_R10] = regs.r10;
  registers[PERF_REG_X86_R11] = regs.r11;
  registers[PERF_REG_X86_R12] = regs.r12;
  registers[PERF_REG_X86_R13] = regs.r13;
  registers[PERF_REG_X86_R14] = regs.r14;
  registers[PERF_REG_X86_R15] = regs.r15;
  return registers;
}

// These cannot be implemented in the header PerfEvent.h, because there
// PerfEventVisitor needs to be an incomplete type to avoid the circular
// dependency between PerfEvent.h and PerfEventVisitor.h.

void ForkPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void ExitPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void ContextSwitchPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void SystemWideContextSwitchPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void StackSamplePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void CallchainSamplePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void UprobesPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void UprobesWithArgumentsPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void UretprobesPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void UretprobesWithReturnValuePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void LostPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void DiscardedPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void MmapPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void TaskNewtaskPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void TaskRenamePerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void SchedSwitchPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void SchedWakeupPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void AmdgpuCsIoctlPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void AmdgpuSchedRunJobPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void DmaFenceSignaledPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

void GenericTracepointPerfEvent::Accept(PerfEventVisitor* visitor) { visitor->Visit(this); }

}  // namespace orbit_linux_tracing
