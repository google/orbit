// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/ApiEventIdProvider.h"

#include <algorithm>
#include <cstdint>

namespace orbit_capture_client {

[[nodiscard]] NameEqualityApiEventIdProvider NameEqualityApiEventIdProvider::Create(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
  const auto instrumented_functions = capture_options.instrumented_functions();
  const uint64_t max_id =
      std::max_element(
          std::begin(instrumented_functions), std::end(instrumented_functions),
          [](const auto& a, const auto& b) { return a.function_id() < b.function_id(); })
          ->function_id();
  return NameEqualityApiEventIdProvider(max_id + 1);
}

NameEqualityApiEventIdProvider::NameEqualityApiEventIdProvider(uint64_t start_id)
    : next_id_(start_id) {}

[[nodiscard]] uint64_t NameEqualityApiEventIdProvider::ProvideId(const TimerInfo& timer_info) {
  const std::string& name = timer_info.api_scope_name();

  const auto it = name_to_id_.find(name);
  if (it != name_to_id_.end()) {
    return it->second;
  }

  name_to_id_[name] = next_id_;
  uint64_t id = next_id_;
  next_id_++;
  return id;
}

}  // namespace orbit_capture_client