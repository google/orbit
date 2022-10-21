// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEE_H_
#define USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEE_H_

#include <sys/types.h>

#include <cstdint>
#include <functional>
#include <memory>

#include "OrbitBase/Result.h"

namespace orbit_user_space_instrumentation {

// Represents a chunk of memory in the tracee.
// The only way to instantiate the class is via the factory function `Create`. We move around
// MemoryInTracee exclusively in unique_ptr. This way we can store instances inside stl containers.
// The memory gets allocated in writable state. In case one wants to execute code in the segment, it
// needs to be made executable with `EnsureMemoryExecutable` later. `Free` deallocates the memory.
// This needs to be done manually - if MemoryInTracee goes out of scope without being freed the
// memory inthe tracee will leak.
// Note that for each of the non-const member functions we need to execute code in the tracee. So we
// need to be attached; the tracee needs to be stopped.
class MemoryInTracee {
 public:
  // We move around MemoryInTracee exclusively in unique_ptr so we don't need it to be copyable or
  // movable. The only way to create a MemoryInTracee is to use the factory function below which
  // uses the private constructor.
  MemoryInTracee() = delete;
  MemoryInTracee(const MemoryInTracee&) = delete;
  MemoryInTracee(MemoryInTracee&&) = delete;
  MemoryInTracee& operator=(const MemoryInTracee&) = delete;
  MemoryInTracee& operator=(MemoryInTracee&&) = delete;

  virtual ~MemoryInTracee() = default;

  // Allocate `size` bytes of memory in the tracee's address space using mmap. The memory will have
  // write permissions. The memory allocated will start at `address`. `address` needs to be aligned
  // to page boundaries. If the memory mapping can not be placed at `address` an error is returned.
  // If `address` is zero the placement of memory will be arbitrary (compare the documentation of
  // mmap: https://man7.org/linux/man-pages/man2/mmap.2.html). Assumes we are already attached to
  // the tracee `pid` using `AttachAndStopProcess`.
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<MemoryInTracee>> Create(pid_t pid,
                                                                              uint64_t address,
                                                                              uint64_t size);

  // Free address range previously allocated with AllocateInTracee using munmap.
  // Assumes we are already attached to the tracee using `AttachAndStopProcess`.
  [[nodiscard]] ErrorMessageOr<void> Free();

  [[nodiscard]] pid_t GetPid() const { return pid_; }
  [[nodiscard]] uint64_t GetAddress() const { return address_; }
  [[nodiscard]] uint64_t GetSize() const { return size_; }
  // Returns the protection state of the memory. The memory gets allocated as writeable and can be
  // made executable (and writable again) with the methods below.
  enum class State { kWritable, kExecutable };
  [[nodiscard]] State GetState() const { return state_; }

  // Sets the read and execute permission for the memory. Removes the write permission. Assumes we
  // are already attached to the tracee using `AttachAndStopProcess`.
  [[nodiscard]] ErrorMessageOr<void> EnsureMemoryExecutable();
  // Set the write permission for the memory. Removes the read and execute permissions. Assumes we
  // are already attached to the tracee using `AttachAndStopProcess`.
  [[nodiscard]] ErrorMessageOr<void> EnsureMemoryWritable();

 protected:
  MemoryInTracee(pid_t pid, uint64_t address, uint64_t size, State state)
      : pid_(pid), address_(address), size_(size), state_(state) {}

  pid_t pid_ = -1;
  uint64_t address_ = 0;
  uint64_t size_ = 0;
  State state_ = State::kWritable;
};

// Same as `MemoryInTracee` above but deallocates memory in the destructor. Note that we still need
// to be attached (or attached again) to the tracee when `AutomaticMemoryInTracee` goes out of
// scope.
class AutomaticMemoryInTracee : public MemoryInTracee {
 public:
  AutomaticMemoryInTracee() = delete;
  AutomaticMemoryInTracee(const AutomaticMemoryInTracee&) = delete;
  AutomaticMemoryInTracee(AutomaticMemoryInTracee&&) = delete;
  AutomaticMemoryInTracee& operator=(const AutomaticMemoryInTracee&) = delete;
  AutomaticMemoryInTracee& operator=(AutomaticMemoryInTracee&&) = delete;

  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<AutomaticMemoryInTracee>> Create(
      pid_t pid, uint64_t address, uint64_t size);

  ~AutomaticMemoryInTracee() override;

 protected:
  using MemoryInTracee::MemoryInTracee;
};

}  // namespace orbit_user_space_instrumentation

#endif  // USER_SPACE_INSTRUMENTATION_ALLOCATE_IN_TRACEE_H_