// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_LIBUNWINDSTACK_UNWINDER_H_
#define LINUX_TRACING_LIBUNWINDSTACK_UNWINDER_H_

#include <asm/perf_regs.h>
#include <unwindstack/Error.h>
#include <unwindstack/MachineX86_64.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Unwinder.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Result.h"
#include "unwindstack/RegsX86_64.h"

namespace orbit_linux_tracing {

class LibunwindstackResult {
 public:
  explicit LibunwindstackResult(
      std::vector<unwindstack::FrameData> frames, unwindstack::RegsX86_64 regs,
      unwindstack::ErrorCode error_code = unwindstack::ErrorCode::ERROR_NONE)
      : frames_{std::move(frames)}, regs_{std::move(regs)}, error_code_{error_code} {}

  [[nodiscard]] const std::vector<unwindstack::FrameData>& frames() const { return frames_; }

  [[nodiscard]] const unwindstack::RegsX86_64& regs() const { return regs_; }

  [[nodiscard]] unwindstack::ErrorCode error_code() const { return error_code_; }

  [[nodiscard]] bool IsSuccess() const { return error_code_ == unwindstack::ErrorCode::ERROR_NONE; }

 private:
  std::vector<unwindstack::FrameData> frames_;
  unwindstack::RegsX86_64 regs_;
  unwindstack::ErrorCode error_code_;
};

class LibunwindstackUnwinder {
 public:
  virtual ~LibunwindstackUnwinder() = default;

  virtual LibunwindstackResult Unwind(pid_t pid, unwindstack::Maps* maps,
                                      const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
                                      const void* stack_dump, uint64_t stack_dump_size,
                                      bool offline_memory_only = false,
                                      size_t max_frames = kDefaultMaxFrames) = 0;

  // Check if, for a given instruction pointer (absolute address), the frame pointer register is
  // set correctly. It may rely on debug information (like Dwarf .debug_frame). Returns an error
  // if the required debug information is not available.
  virtual std::optional<bool> HasFramePointerSet(uint64_t instruction_pointer, pid_t pid,
                                                 unwindstack::Maps* maps) = 0;

  static std::unique_ptr<LibunwindstackUnwinder> Create();
  static std::string LibunwindstackErrorString(unwindstack::ErrorCode error_code);

 protected:
  static constexpr size_t kDefaultMaxFrames = 1024;  // This is arbitrary.
};
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LIBUNWINDSTACK_UNWINDER_H_
