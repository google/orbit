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

template <typename>
[[maybe_unused]] static constexpr bool always_false_v = false;

void PerfEvent::Accept(PerfEventVisitor* visitor) {
  std::visit(
      [this, visitor](auto&& event_data) {
        using PerfEventDataT = std::decay_t<decltype(event_data)>;

        if constexpr (std::is_same_v<PerfEventDataT, ForkPerfEventData>) {
          visitor->Visit(static_cast<ForkPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, ExitPerfEventData>) {
          visitor->Visit(static_cast<ExitPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, LostPerfEventData>) {
          visitor->Visit(static_cast<LostPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, DiscardedPerfEventData>) {
          visitor->Visit(static_cast<DiscardedPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, StackSamplePerfEventData>) {
          visitor->Visit(static_cast<StackSamplePerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, CallchainSamplePerfEventData>) {
          visitor->Visit(static_cast<CallchainSamplePerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, UprobesPerfEventData>) {
          visitor->Visit(static_cast<UprobesPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, UprobesWithArgumentsPerfEventData>) {
          visitor->Visit(static_cast<UprobesWithArgumentsPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, UretprobesPerfEventData>) {
          visitor->Visit(static_cast<UretprobesPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT,
                                            UretprobesWithReturnValuePerfEventData>) {
          visitor->Visit(static_cast<UretprobesWithReturnValuePerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, MmapPerfEventData>) {
          visitor->Visit(static_cast<MmapPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, GenericTracepointPerfEventData>) {
          visitor->Visit(static_cast<GenericTracepointPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, TaskNewtaskPerfEventData>) {
          visitor->Visit(static_cast<TaskNewtaskPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, TaskRenamePerfEventData>) {
          visitor->Visit(static_cast<TaskRenamePerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, SchedSwitchPerfEventData>) {
          visitor->Visit(static_cast<SchedSwitchPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, SchedWakeupPerfEventData>) {
          visitor->Visit(static_cast<SchedWakeupPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, AmdgpuCsIoctlPerfEventData>) {
          visitor->Visit(static_cast<AmdgpuCsIoctlPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, AmdgpuSchedRunJobPerfEventData>) {
          visitor->Visit(static_cast<AmdgpuSchedRunJobPerfEvent*>(this));
        } else if constexpr (std::is_same_v<PerfEventDataT, DmaFenceSignaledPerfEventData>) {
          visitor->Visit(static_cast<DmaFenceSignaledPerfEvent*>(this));
        } else {
          static_assert(always_false_v<PerfEventDataT>, "Non-exhaustive visitor");
        }
      },
      this->data_variant());
}

}  // namespace orbit_linux_tracing
