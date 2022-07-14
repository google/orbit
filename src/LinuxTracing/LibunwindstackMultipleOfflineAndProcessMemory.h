// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LIBUNWINDSTACK_MULTIPLE_OFFLINE_AND_PROCESS_MEMORY_H_
#define LINUX_TRACING_LIBUNWINDSTACK_MULTIPLE_OFFLINE_AND_PROCESS_MEMORY_H_

#include <absl/base/casts.h>
#include <unwindstack/Memory.h>

#include <memory>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing {

// This class is a "view" of a stack slice (some copy of process memory). It contains a bare pointer
// to the array that actually owns the stack data.
class StackSliceView {
 public:
  StackSliceView(uint64_t start_address, uint64_t size, const char* data)
      : start_address_{start_address}, size_{size}, data_{data} {}

  [[nodiscard]] uint64_t start_address() const { return start_address_; }
  [[nodiscard]] uint64_t end_address() const { return start_address_ + size_; }
  [[nodiscard]] uint64_t size() const { return size_; }
  [[nodiscard]] const char* data() const { return data_; }

 private:
  uint64_t start_address_;
  uint64_t size_;
  const char* data_;
};

// This class is a thin layer around unwindstack::MemoryOfflineBuffer, that allows querying address
// and size of the underlying offline memory as well as specifying the memory region as
// `StackSliceView`.
class LibunwindstackOfflineMemory : public unwindstack::Memory {
 public:
  LibunwindstackOfflineMemory(StackSliceView stack_slice_view)
      : stack_slice_view_{stack_slice_view},
        memory_{unwindstack::Memory::CreateOfflineMemory(
            absl::bit_cast<const uint8_t*>(stack_slice_view.data()),
            stack_slice_view.start_address(), stack_slice_view.end_address())} {};
  size_t Read(uint64_t addr, void* dst, size_t size) override {
    return memory_->Read(addr, dst, size);
  }

  [[nodiscard]] virtual uint64_t start_address() const { return stack_slice_view_.start_address(); }
  [[nodiscard]] virtual uint64_t end_address() const { return stack_slice_view_.end_address(); }
  [[nodiscard]] virtual uint64_t size() const { return stack_slice_view_.size(); }

 private:
  StackSliceView stack_slice_view_;
  std::shared_ptr<unwindstack::Memory> memory_;
};

// This custom implementation of `unwindstack::Memory` carries multiple stack slices, each same as
// `CreateOfflineMemory` would. When requesting to read an address range, the class will go through
// the stack slices and if one slice fully contains the requested address range it will read from
// this stack slice. When requesting an address range outside any of the stack samples,
// the class falls back to reading from the memory of the process online, as `CreateProcessMemory`
// would.
// If the process memory is not specified, the fall back to reading process memory will not be done.
// Having multiple stack slices allows unwinding callstacks that have multiple stacks involved, such
// as in the case of Wine system calls.
// The process memory allows unwinding callstacks that involve virtual modules, such as vDSO.
class LibunwindstackMultipleOfflineAndProcessMemory : public unwindstack::Memory {
 public:
  size_t Read(uint64_t addr, void* dst, size_t size) override;

  LibunwindstackMultipleOfflineAndProcessMemory(
      std::shared_ptr<Memory> process_memory,
      std::vector<LibunwindstackOfflineMemory> stack_memory_slices)
      : process_memory_{std::move(process_memory)},
        stack_memory_slices_{std::move(std::move(stack_memory_slices))} {}

  static std::shared_ptr<Memory> Create(pid_t pid,
                                        const std::vector<StackSliceView>& stack_slices) {
    std::vector<LibunwindstackOfflineMemory> stack_memory_slices =
        CreateOfflineMemorySlices(stack_slices);
    return std::make_shared<LibunwindstackMultipleOfflineAndProcessMemory>(
        unwindstack::Memory::CreateProcessMemoryCached(pid), std::move(stack_memory_slices));
  }

  static std::shared_ptr<Memory> Create(const std::vector<StackSliceView>& stack_slices) {
    std::vector<LibunwindstackOfflineMemory> stack_memory_slices =
        CreateOfflineMemorySlices(stack_slices);
    return std::make_shared<LibunwindstackMultipleOfflineAndProcessMemory>(
        nullptr, std::move(stack_memory_slices));
  }

 private:
  static std::vector<LibunwindstackOfflineMemory> CreateOfflineMemorySlices(
      const std::vector<StackSliceView>& stack_slices);

  std::shared_ptr<Memory> process_memory_;
  std::vector<LibunwindstackOfflineMemory> stack_memory_slices_;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LIBUNWINDSTACK_MULTIPLE_OFFLINE_AND_PROCESS_MEMORY_H_
