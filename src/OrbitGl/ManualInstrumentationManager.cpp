// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ManualInstrumentationManager.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/meta/type_traits.h>
#include <absl/synchronization/mutex.h>
#include <stddef.h>

#include "OrbitBase/Logging.h"

using orbit_client_protos::TimerInfo;

void ManualInstrumentationManager::AddAsyncTimerListener(AsyncTimerInfoListener* listener) {
  absl::MutexLock lock(&mutex_);
  async_timer_info_listeners_.insert(listener);
}

void ManualInstrumentationManager::RemoveAsyncTimerListener(AsyncTimerInfoListener* listener) {
  absl::MutexLock lock(&mutex_);
  async_timer_info_listeners_.erase(listener);
}

orbit_api::Event ManualInstrumentationManager::ApiEventFromTimerInfo(
    const orbit_client_protos::TimerInfo& timer_info) {
  // On x64 Linux, 6 registers are used for integer argument passing.
  // Manual instrumentation uses those registers to encode orbit_api::Event
  // objects.
  constexpr size_t kNumIntegerRegisters = 6;
  CHECK(timer_info.registers_size() == kNumIntegerRegisters);
  uint64_t arg_0 = timer_info.registers(0);
  uint64_t arg_1 = timer_info.registers(1);
  uint64_t arg_2 = timer_info.registers(2);
  uint64_t arg_3 = timer_info.registers(3);
  uint64_t arg_4 = timer_info.registers(4);
  uint64_t arg_5 = timer_info.registers(5);
  orbit_api::EncodedEvent encoded_event(arg_0, arg_1, arg_2, arg_3, arg_4, arg_5);
  return encoded_event.event;
}

void ManualInstrumentationManager::ProcessAsyncTimerDeprecated(
    const orbit_client_protos::TimerInfo& timer_info) {
  orbit_api::Event event = ApiEventFromTimerInfo(timer_info);
  const uint64_t event_id = event.data;
  if (event.type == orbit_api::kScopeStartAsync) {
    async_timer_info_start_by_id_[event_id] = timer_info;
  } else if (event.type == orbit_api::kScopeStopAsync) {
    auto it = async_timer_info_start_by_id_.find(event_id);
    if (it != async_timer_info_start_by_id_.end()) {
      const TimerInfo& start_timer_info = it->second;
      orbit_api::Event start_event = ApiEventFromTimerInfo(start_timer_info);

      TimerInfo async_span = start_timer_info;
      async_span.set_end(timer_info.end());
      absl::MutexLock lock(&mutex_);
      for (auto* listener : async_timer_info_listeners_) (*listener)(start_event.name, async_span);
    }
  }
}

void ManualInstrumentationManager::ProcessAsyncTimer(
    const orbit_client_protos::TimerInfo& timer_info) {
  orbit_api::Event start_event = ApiEventFromTimerInfo(timer_info);
  absl::MutexLock lock(&mutex_);
  for (auto* listener : async_timer_info_listeners_) {
    (*listener)(start_event.name, timer_info);
  }
}

void ManualInstrumentationManager::ProcessStringEvent(const orbit_api::Event& event) {
  // A string can be sent in chunks so we append the current value to any existing one.
  const uint64_t event_id = event.data;
  auto result = string_manager_.Get(event_id);
  if (result.has_value()) {
    string_manager_.AddOrReplace(event_id, result.value() + event.name);
  } else {
    string_manager_.AddOrReplace(event_id, event.name);
  }
}
