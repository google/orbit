// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibunwindstackMultipleOfflineAndProcessMemory.h"

#include <absl/base/casts.h>

namespace orbit_linux_tracing {
size_t LibunwindstackMultipleOfflineAndProcessMemory::Read(uint64_t addr, void* dst, size_t size) {
  const uint64_t addr_start = addr;
  const uint64_t addr_end = addr + size;

  bool found_partial_intersection = false;

  // If the requested address range is entirely in the stack sample's address range, read from the
  // stack buffer.
  for (LibunwindstackOfflineMemory& stack_memory : stack_memories_) {
    if (addr_start >= stack_memory.start_address() && addr_end <= stack_memory.end_address()) {
      return stack_memory.Read(addr, dst, size);
    }

    // If the requested address range is partially intersecting with the stack slice, something went
    // wrong. However, there could be another stack slice that entirely contains the requested
    // address range later, so don't give up already.
    if (addr_end > stack_memory.start_address() && addr_start < stack_memory.end_address()) {
      found_partial_intersection = true;
    }
  }

  // The requested address range is partially intersecting with at least one stack slice, but we
  // don't have the offline memory for the complete range. Something went wrong, so don't read any
  // data.
  if (found_partial_intersection) {
    return 0;
  }

  // If the requested range is entirely disjoint from the stack slices' address range read from
  // the memory of the process.
  if (process_memory_ != nullptr) {
    return process_memory_->Read(addr, dst, size);
  }

  return 0;
}

std::vector<LibunwindstackOfflineMemory>
LibunwindstackMultipleOfflineAndProcessMemory::CreateOfflineMemorySlices(
    const std::vector<StackSliceView>& stack_slices) {
  std::vector<LibunwindstackOfflineMemory> stack_memory_slices{};
  stack_memory_slices.reserve(stack_slices.size());
  for (const StackSliceView& stack_slice_view : stack_slices) {
    stack_memory_slices.emplace_back(stack_slice_view);
  }
  return stack_memory_slices;
}
}  // namespace orbit_linux_tracing
