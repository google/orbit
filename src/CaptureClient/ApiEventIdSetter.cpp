// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/ApiEventIdSetter.h"

#include <cstdint>

[[nodiscard]] static bool IsApiScope(const TimerInfo& timer_info) {
  return timer_info.type() == TimerInfo::kApiScope ||
         timer_info.type() == TimerInfo::kApiScopeAsync;
}

namespace orbit_capture_client {

ApiEventIdSetter ApiEventIdSetter::Create(
    const ::orbit_grpc_protos::CaptureOptions& capture_options) {
  uint64_t max_id = 0;
  for (const auto& instrumented_function : capture_options.instrumented_functions()) {
    max_id = std::max(max_id, instrumented_function.function_id());
  }
  return ApiEventIdSetter(max_id + 1);
}

ApiEventIdSetter::ApiEventIdSetter(uint64_t start_id) : next_id_(start_id) {}

void ApiEventIdSetter::SetId(TimerInfo& timer_info) {
  if (!IsApiScope(timer_info)) return;

  const std::string& name = timer_info.api_scope_name();
  auto it = name_to_id_.find(name);
  uint64_t id{};
  if (it != name_to_id_.end()) {
    id = it->second;
  } else {
    name_to_id_.insert_or_assign(name, next_id_++);
  }
  timer_info.set_api_scope_group_id(id);
}

}  // namespace orbit_capture_client