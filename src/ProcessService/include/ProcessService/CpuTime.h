// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PROCESS_SERVICE_CPU_TIME_H_
#define PROCESS_SERVICE_CPU_TIME_H_

#include <cstddef>
#include <cstdint>

namespace orbit_process_service_internal {

// In the linux world, Jiffies is a global counter which increments on tick (caused by a CPU timer
// interrupt). This struct is a poor man's strong type to ensure that this measure is not mistakenly
// interpreted as nanoseconds.
struct Jiffies {
  uint64_t value;
};

struct TotalCpuTime {
  // jiffies is the sum over all cycles executed on all cores.
  Jiffies jiffies;

  // cpus is the number of (logical) cores available and accumulated in jiffies.
  size_t cpus;
};

}  // namespace orbit_process_service_internal

#endif  // PROCESS_SERVICE_CPU_TIME_H_
