// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackUnwinder.h"

#include <OrbitBase/Logging.h>

#include <array>

namespace LinuxTracing {

std::unique_ptr<unwindstack::BufferMaps> LibunwindstackUnwinder::ParseMaps(
    const std::string& maps_buffer) {
  auto maps = std::make_unique<unwindstack::BufferMaps>(maps_buffer.c_str());
  if (!maps->Parse()) {
    return nullptr;
  }
  return maps;
}

const std::array<size_t, unwindstack::X86_64_REG_LAST>
    LibunwindstackUnwinder::UNWINDSTACK_REGS_TO_PERF_REGS{
        PERF_REG_X86_AX,  PERF_REG_X86_DX,  PERF_REG_X86_CX,  PERF_REG_X86_BX,
        PERF_REG_X86_SI,  PERF_REG_X86_DI,  PERF_REG_X86_BP,  PERF_REG_X86_SP,
        PERF_REG_X86_R8,  PERF_REG_X86_R9,  PERF_REG_X86_R10, PERF_REG_X86_R11,
        PERF_REG_X86_R12, PERF_REG_X86_R13, PERF_REG_X86_R14, PERF_REG_X86_R15,
        PERF_REG_X86_IP,
    };

std::vector<unwindstack::FrameData> LibunwindstackUnwinder::Unwind(
    unwindstack::Maps* maps,
    const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
    const void* stack_dump, uint64_t stack_dump_size) {
  unwindstack::RegsX86_64 regs{};
  for (size_t perf_reg = 0; perf_reg < unwindstack::X86_64_REG_LAST;
       ++perf_reg) {
    regs[perf_reg] = perf_regs.at(UNWINDSTACK_REGS_TO_PERF_REGS[perf_reg]);
  }

  std::shared_ptr<unwindstack::Memory> memory =
      unwindstack::Memory::CreateOfflineMemory(
          static_cast<const uint8_t*>(stack_dump),
          regs[unwindstack::X86_64_REG_RSP],
          regs[unwindstack::X86_64_REG_RSP] + stack_dump_size);

  unwindstack::Unwinder unwinder{MAX_FRAMES, maps, &regs, memory};
  // Careful: regs are modified. Use regs.Clone() if you need to reuse regs
  // later.
  unwinder.Unwind();

  // Samples that fall inside a function dynamically-instrumented with
  // uretprobes often result in unwinding errors when hitting the trampoline
  // inserted by the uretprobe. Do not treat them as errors as we might want
  // those callstacks.
  if (unwinder.LastErrorCode() != 0 &&
      unwinder.frames().back().map_name != "[uprobes]") {
#ifndef NDEBUG
    ERROR("%s at %#016lx",
          LibunwindstackErrorString(unwinder.LastErrorCode()).c_str(),
          unwinder.LastErrorAddress());
#endif
    return {};
  }

  return unwinder.frames();
}

}  // namespace LinuxTracing
