// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_MANUAL_INSTRUMENTATION_CONFIG_H_
#define LINUX_TRACING_MANUAL_INSTRUMENTATION_CONFIG_H_

#include <absl/container/flat_hash_set.h>

#include <cstdint>

#include "Function.h"

namespace orbit_linux_tracing {
class ManualInstrumentationConfig {
 public:
  void AddTimerStartFunctionId(uint64_t id) { timer_start_function_ids_.insert(id); }

  void AddTimerStopFunctionId(uint64_t id) { timer_stop_function_ids_.insert(id); }

  [[nodiscard]] bool IsTimerStartFunction(uint64_t function_id) const {
    return timer_start_function_ids_.contains(function_id);
  }

  [[nodiscard]] bool IsTimerStopFunction(uint64_t function_id) const {
    return timer_stop_function_ids_.contains(function_id);
  }

 private:
  absl::flat_hash_set<uint64_t> timer_start_function_ids_;
  absl::flat_hash_set<uint64_t> timer_stop_function_ids_;
};
}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_MANUAL_INSTRUMENTATION_CONFIG_H_
