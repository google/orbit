// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_
#define ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_

#include "../Orbit.h"
#include "OrbitBase/Logging.h"
#include "capture_data.pb.h"

class ManualInstrumentationManager {
 public:
  [[nodiscard]] static orbit_api::Event ApiEventFromTimerInfo(
      const orbit_client_protos::TimerInfo& timer_info) {
    // On x64 Linux, 6 registers are used for integer argument passing.
    // Manual instrumentation uses those registers to encode orbit_api::Event
    // objects.
    constexpr size_t kNumIntegerRegisers = 6;
    CHECK(timer_info.registers_size() == kNumIntegerRegisers);
    uint64_t arg_0 = timer_info.registers(0);
    uint64_t arg_1 = timer_info.registers(1);
    uint64_t arg_2 = timer_info.registers(2);
    uint64_t arg_3 = timer_info.registers(3);
    uint64_t arg_4 = timer_info.registers(4);
    uint64_t arg_5 = timer_info.registers(5);
    orbit_api::EncodedEvent encoded_event(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
    return encoded_event.event;
  }
};

#endif  // ORBIT_GL_MANUAL_INSTRUMENTATION_MANAGER_H_
