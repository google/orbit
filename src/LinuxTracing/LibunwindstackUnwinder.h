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
#include <vector>

namespace orbit_linux_tracing {

class LibunwindstackUnwinder {
 public:
  virtual ~LibunwindstackUnwinder() = default;

  virtual std::vector<unwindstack::FrameData> Unwind(
      pid_t pid, unwindstack::Maps* maps,
      const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs, const void* stack_dump,
      uint64_t stack_dump_size) = 0;

  static std::unique_ptr<LibunwindstackUnwinder> Create();
};
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_LIBUNWINDSTACK_UNWINDER_H_
