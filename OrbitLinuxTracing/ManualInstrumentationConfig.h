// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_MANUAL_INSTRUMENTATION_CONFIG_H_
#define ORBIT_LINUX_TRACING_MANUAL_INSTRUMENTATION_CONFIG_H_

#include <cstdint>

#include "absl/container/flat_hash_set.h"

class ManualInstrumentationConfig {
 public:
  inline void AddTimerStartAddress(uint64_t address) {
    timer_start_addresses_.insert(address);
  }

  inline void AddTimerStopAddress(uint64_t address) {
    timer_stop_addresses_.insert(address);
  }

  inline bool IsTimerStartAddress(uint64_t address) const {
    return timer_start_addresses_.contains(address);
  }

  inline bool IsTimerStopAddress(uint64_t address) const {
    return timer_stop_addresses_.contains(address);
  }

 private:
  absl::flat_hash_set<uint64_t> timer_start_addresses_;
  absl::flat_hash_set<uint64_t> timer_stop_addresses_;
};

#endif
