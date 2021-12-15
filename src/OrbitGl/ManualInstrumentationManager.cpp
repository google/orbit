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

void ManualInstrumentationManager::ProcessStringEvent(
    const orbit_client_protos::ApiStringEvent& string_event) {
  const uint64_t event_id = string_event.async_scope_id();
  if (!string_event.should_concatenate()) {
    string_manager_.AddOrReplace(event_id, string_event.name());
  } else {
    // In the legacy implementation of manual instrumentation, a string could be sent in chunks, so
    // in that case we append the current value to any existing one.
    std::string previous_string = string_manager_.Get(event_id).value_or("");
    string_manager_.AddOrReplace(event_id, previous_string + string_event.name());
  }
}

void ManualInstrumentationManager::ProcessAsyncTimer(const TimerInfo& timer_info) {
  absl::MutexLock lock(&mutex_);
  for (auto* listener : async_timer_info_listeners_) {
    (*listener)(timer_info.api_scope_name(), timer_info);
  }
}
