// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/ManualInstrumentationManager.h"

void ManualInstrumentationManager::ProcessStringEvent(
    const orbit_client_data::ApiStringEvent& string_event) {
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
