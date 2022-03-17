// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/ApiEventIdSetter.h"

#include <algorithm>
#include <cstdint>

[[nodiscard]] static bool IsApiScope(const TimerInfo& timer_info) {
  return timer_info.type() == TimerInfo::kApiScope ||
         timer_info.type() == TimerInfo::kApiScopeAsync;
}

namespace orbit_capture_client {

void ApiEventIdSetter::SetId(TimerInfo& timer_info) {
  if (!IsApiScope(timer_info)) return;
  timer_info.set_api_scope_group_id(GetId(timer_info));
}

[[nodiscard]] NameEqualityApiEventIdSetter NameEqualityApiEventIdSetter::Create(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
  const auto instrumented_functions = capture_options.instrumented_functions();
  int max_id = std::max_element(
                   std::begin(instrumented_functions), std::end(instrumented_functions),
                   [](const auto& a, const auto& b) { return a.function_id() < b.function_id(); })
                   ->function_id();
  return NameEqualityApiEventIdSetter(max_id + 1);
}

NameEqualityApiEventIdSetter::NameEqualityApiEventIdSetter(uint64_t start_id)
    : next_id_(start_id) {}

uint64_t NameEqualityApiEventIdSetter::GetId(const TimerInfo& timer_info) {
  const std::string& name = timer_info.api_scope_name();

  const auto it = name_to_id_.find(name);
  if (it != name_to_id_.end()) {
    return it->second;
  }

  name_to_id_.insert_or_assign(name, next_id_);
  uint64_t id = next_id_;
  next_id_++;
  return id;
}

}  // namespace orbit_capture_client