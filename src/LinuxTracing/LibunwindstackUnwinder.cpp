// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackUnwinder.h"

#include <unwindstack/Error.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsX86_64.h>

#include <array>
#include <cstdint>
#include <map>

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
                              const void* stack_dump, uint64_t stack_dump_size,
                              bool offline_memory_only = false,
                              size_t max_frames = kDefaultMaxFrames) override;

  std::optional<bool> HasFramePointerSet(uint64_t instruction_pointer,
                                         unwindstack::Maps* maps) override;

 private:
  static const std::array<size_t, unwindstack::X86_64_REG_LAST> kUnwindstackRegsToPerfRegs;

  std::map<uint64_t, unwindstack::DwarfLocations>
      debug_frame_loc_regs_cache_;  // Single row indexed by pc_end.
  std::map<uint64_t, unwindstack::DwarfLocations>
      eh_frame_loc_regs_cache_;  // Single row indexed by pc_end.
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
    const void* stack_dump, uint64_t stack_dump_size, bool offline_memory_only, size_t max_frames) {
  unwindstack::RegsX86_64 regs{};
  for (size_t perf_reg = 0; perf_reg < unwindstack::X86_64_REG_LAST; ++perf_reg) {
    regs[perf_reg] = perf_regs.at(kUnwindstackRegsToPerfRegs[perf_reg]);
  }

  std::shared_ptr<unwindstack::Memory> memory = nullptr;
  if (offline_memory_only) {
    memory = unwindstack::Memory::CreateOfflineMemory(
        static_cast<const uint8_t*>(stack_dump), regs[unwindstack::X86_64_REG_RSP],
        regs[unwindstack::X86_64_REG_RSP] + stack_dump_size);
  } else {
    memory = StackAndProcessMemory::Create(pid, static_cast<const uint8_t*>(stack_dump),
                                           regs[unwindstack::X86_64_REG_RSP],
                                           regs[unwindstack::X86_64_REG_RSP] + stack_dump_size);
  }

  unwindstack::Unwinder unwinder{max_frames, maps, &regs, memory};
  // Careful: regs are modified. Use regs.Clone() if you need to reuse regs later.
  unwinder.Unwind();

#ifndef NDEBUG
  if (unwinder.LastErrorCode() != 0) {
    ORBIT_ERROR("%s at %#016lx", LibunwindstackErrorString(unwinder.LastErrorCode()),
                unwinder.LastErrorAddress());
  }
#endif

  return LibunwindstackResult{unwinder.ConsumeFrames(), unwinder.LastErrorCode()};
}
// This functions detects if a frame pointer register was set in the given program counter using
// the given Dwarf section.
// It does so by verifying if "Canonical Frame Address" gets computed immediately from $rbp (with
// offset 16 to skip the old frame pointer and the return address).
// The function returns nullopt if the required Dwarf information is not available.
static std::optional<bool> HasFramePointerSetFromDwarfSection(
    uint64_t rel_pc, unwindstack::DwarfSection* dwarf_section,
    std::map<uint64_t, unwindstack::DwarfLocations>* loc_regs_cache) {
  if (dwarf_section == nullptr) {
    return false;
  }

  auto cache_it = loc_regs_cache->upper_bound(rel_pc);
  if (cache_it == loc_regs_cache->end() || rel_pc < cache_it->second.pc_start) {
    const unwindstack::DwarfFde* fde = dwarf_section->GetFdeFromPc(rel_pc);
    if (fde == nullptr) {
      return std::nullopt;
    }
    unwindstack::DwarfLocations loc_regs;
    if (!dwarf_section->GetCfaLocationInfo(rel_pc, fde, &loc_regs, unwindstack::ARCH_X86_64)) {
      return std::nullopt;
    }
    cache_it = loc_regs_cache->emplace(loc_regs.pc_end, std::move(loc_regs)).first;
  }

  unwindstack::DwarfLocations* loc_regs = &cache_it->second;
  auto cfa_entry = loc_regs->find(unwindstack::CFA_REG);
  if (cfa_entry == loc_regs->end()) {
    return false;
  }
  const unwindstack::DwarfLocation* loc = &cfa_entry->second;
  ORBIT_CHECK(loc != nullptr);
  if (loc->type == unwindstack::DWARF_LOCATION_REGISTER) {
    // From the Dwarf standard:
    //  "Typically, the CFA is defined to be the value of the stack pointer at the call site in the
    //  previous frame (which may be different from its value on entry to the current frame)"
    // So for the frame pointer case, the "value of the stack pointer at the call site" is:
    // $rbp + 8 (for the prev. frame pointer) + 8 (for the return address)
    if (loc->values[0] == unwindstack::X86_64_REG_RBP && loc->values[1] == 16) {
      return true;
    }
  }

  return false;
}

std::optional<bool> LibunwindstackUnwinderImpl::HasFramePointerSet(uint64_t instruction_pointer,
                                                                   unwindstack::Maps* maps) {
  unwindstack::MapInfo* map_info = maps->Find(instruction_pointer);
  if (map_info == nullptr) {
    return std::nullopt;
  }

  // We don't have enough process memory collected to read the object file from memory (if memory
  // mapped), so we don't even try and directly read from file.
  unwindstack::Object* object = map_info->GetObject(nullptr, unwindstack::ARCH_X86_64);
  if (object == nullptr) {
    return std::nullopt;
  }

  auto* elf = dynamic_cast<unwindstack::Elf*>(object);
  if (elf == nullptr) {
    // TODO(b/228599622): Handle the PeCoff case.
    return false;
  }

  if (!elf->valid()) {
    return std::nullopt;
  }

  uint64_t rel_pc = object->GetRelPc(instruction_pointer, map_info);

  unwindstack::DwarfSection* debug_frame = elf->interface()->debug_frame();

  auto has_frame_pointer_set_from_debug_frame_or_error =
      orbit_linux_tracing::HasFramePointerSetFromDwarfSection(rel_pc, debug_frame,
                                                              &debug_frame_loc_regs_cache_);
  if (!has_frame_pointer_set_from_debug_frame_or_error.has_value()) {
    return std::nullopt;
  }
  if (*has_frame_pointer_set_from_debug_frame_or_error) {
    return true;
  }

  unwindstack::DwarfSection* eh_frame = elf->interface()->eh_frame();
  auto has_frame_pointer_set_from_eh_frame_or_error =
      orbit_linux_tracing::HasFramePointerSetFromDwarfSection(rel_pc, eh_frame,
                                                              &eh_frame_loc_regs_cache_);
  if (!has_frame_pointer_set_from_eh_frame_or_error.has_value()) {
    return std::nullopt;
  }
  return *has_frame_pointer_set_from_eh_frame_or_error;
}
}  // namespace

std::unique_ptr<LibunwindstackUnwinder> LibunwindstackUnwinder::Create() {
  return std::make_unique<LibunwindstackUnwinderImpl>();
}

std::string LibunwindstackUnwinder::LibunwindstackErrorString(unwindstack::ErrorCode error_code) {
  return std::string(unwindstack::GetErrorCodeString(error_code));
}

}  // namespace orbit_linux_tracing
