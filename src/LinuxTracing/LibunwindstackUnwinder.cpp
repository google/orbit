// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackUnwinder.h"

#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsX86_64.h>

#include <array>
#include <cstdint>

#include "OrbitBase/Logging.h"  // IWYU pragma: keep

namespace orbit_linux_tracing {

namespace {

// This custom implementation of `unwindstack::Memory` carries a stack sample as
// `CreateOfflineMemory` would, but when requesting an address range outside of the stack sample it
// falls back to reading from the memory of the process online, as `CreateProcessMemory` would.
// This allows to unwind callstacks that involve virtual modules, such as vDSO.
class StackAndProcessMemory : public unwindstack::Memory {
 public:
  size_t Read(uint64_t addr, void* dst, size_t size) override {
    const uint64_t addr_start = addr;
    const uint64_t addr_end = addr + size;

    // If the requested address range is entirely in the stack sample's address range, read from the
    // stack buffer.
    if (addr_start >= stack_start_ && addr_end <= stack_end_) {
      return stack_memory_->Read(addr, dst, size);
    }

    // If the requested address range is entirely disjoint from the stack sample's address range,
    // read from the memory of the process.
    if (addr_end <= stack_start_ || addr_start >= stack_end_) {
      return process_memory_->Read(addr, dst, size);
    }

    // Otherwise, something is probably wrong with the address range requested. Don't read anything.
    return 0;
  }

  static std::shared_ptr<Memory> Create(pid_t pid, const uint8_t* stack_data, uint64_t stack_start,
                                        uint64_t stack_end) {
    return std::shared_ptr<StackAndProcessMemory>(
        new StackAndProcessMemory(pid, stack_data, stack_start, stack_end));
  }

 private:
  StackAndProcessMemory(pid_t pid, const uint8_t* stack_data, uint64_t stack_start,
                        uint64_t stack_end)
      : process_memory_{unwindstack::Memory::CreateProcessMemoryCached(pid)},
        stack_memory_{unwindstack::Memory::CreateOfflineMemory(stack_data, stack_start, stack_end)},
        stack_start_{stack_start},
        stack_end_{stack_end} {}

  std::shared_ptr<Memory> process_memory_;
  std::shared_ptr<Memory> stack_memory_;
  uint64_t stack_start_;
  uint64_t stack_end_;
};

class LibunwindstackUnwinderImpl : public LibunwindstackUnwinder {
 public:
  LibunwindstackResult Unwind(pid_t pid, unwindstack::Maps* maps,
                              const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
                              const void* stack_dump, uint64_t stack_dump_size) override;

 private:
  static constexpr size_t kMaxFrames = 1024;  // This is arbitrary.

  static const std::array<size_t, unwindstack::X86_64_REG_LAST> kUnwindstackRegsToPerfRegs;

  static std::string LibunwindstackErrorString(unwindstack::ErrorCode error_code) {
    static const std::vector<const char*> kErrorNames{
        "ERROR_NONE",           "ERROR_MEMORY_INVALID", "ERROR_UNWIND_INFO",
        "ERROR_UNSUPPORTED",    "ERROR_INVALID_MAP",    "ERROR_MAX_FRAMES_EXCEEDED",
        "ERROR_REPEATED_FRAME", "ERROR_INVALID_ELF"};
    return kErrorNames[error_code];
  }
};

const std::array<size_t, unwindstack::X86_64_REG_LAST>
    LibunwindstackUnwinderImpl::kUnwindstackRegsToPerfRegs{
        PERF_REG_X86_AX,  PERF_REG_X86_DX,  PERF_REG_X86_CX,  PERF_REG_X86_BX,  PERF_REG_X86_SI,
        PERF_REG_X86_DI,  PERF_REG_X86_BP,  PERF_REG_X86_SP,  PERF_REG_X86_R8,  PERF_REG_X86_R9,
        PERF_REG_X86_R10, PERF_REG_X86_R11, PERF_REG_X86_R12, PERF_REG_X86_R13, PERF_REG_X86_R14,
        PERF_REG_X86_R15, PERF_REG_X86_IP,
    };

LibunwindstackResult LibunwindstackUnwinderImpl::Unwind(
    pid_t pid, unwindstack::Maps* maps, const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
    const void* stack_dump, uint64_t stack_dump_size) {
  unwindstack::RegsX86_64 regs{};
  for (size_t perf_reg = 0; perf_reg < unwindstack::X86_64_REG_LAST; ++perf_reg) {
    regs[perf_reg] = perf_regs.at(kUnwindstackRegsToPerfRegs[perf_reg]);
  }

  std::shared_ptr<unwindstack::Memory> memory = StackAndProcessMemory::Create(
      pid, static_cast<const uint8_t*>(stack_dump), regs[unwindstack::X86_64_REG_RSP],
      regs[unwindstack::X86_64_REG_RSP] + stack_dump_size);

  unwindstack::Unwinder unwinder{kMaxFrames, maps, &regs, memory};
  // Careful: regs are modified. Use regs.Clone() if you need to reuse regs later.
  unwinder.Unwind();

#ifndef NDEBUG
  if (unwinder.LastErrorCode() != 0) {
    ERROR("%s at %#016lx", LibunwindstackErrorString(unwinder.LastErrorCode()),
          unwinder.LastErrorAddress());
  }
#endif

  return LibunwindstackResult{unwinder.ConsumeFrames(), unwinder.LastErrorCode()};
}

}  // namespace

std::unique_ptr<LibunwindstackUnwinder> LibunwindstackUnwinder::Create() {
  return std::make_unique<LibunwindstackUnwinderImpl>();
}

}  // namespace orbit_linux_tracing
