// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ScopeIdProvider.h"

#include <absl/container/flat_hash_map.h>
#include <absl/flags/flag.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "ClientData/ScopeIdConstants.h"
#include "ClientFlags/ClientFlags.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

std::unique_ptr<NameEqualityScopeIdProvider> NameEqualityScopeIdProvider::Create(
    const orbit_grpc_protos::CaptureOptions& capture_options) {
  const auto& instrumented_functions = capture_options.instrumented_functions();
  const uint64_t max_id =
      instrumented_functions.empty()
          ? 0
          : std::max_element(
                std::begin(instrumented_functions), std::end(instrumented_functions),
                [](const auto& a, const auto& b) { return a.function_id() < b.function_id(); })
                ->function_id();

  absl::flat_hash_map<uint64_t, std::string> scope_id_to_name;
  for (const auto& instrumented_function : instrumented_functions) {
    scope_id_to_name[instrumented_function.function_id()] = instrumented_function.function_name();
  }

  return std::unique_ptr<NameEqualityScopeIdProvider>(
      new NameEqualityScopeIdProvider(max_id + 1, std::move(scope_id_to_name)));
}

uint64_t NameEqualityScopeIdProvider::FunctionIdToScopeId(uint64_t function_id) const {
  return function_id;
}

uint64_t NameEqualityScopeIdProvider::ProvideId(const TimerInfo& timer_info) {
  // Check if the `timer_info` corresponds to a hooked function event. Checking for `function_id`
  // not being invalid is not sufficient, as e.g. frametrack events may also have non-invalid
  // function_id
  if (timer_info.function_id() != orbit_grpc_protos::kInvalidFunctionId &&
      timer_info.type() == orbit_client_protos::TimerInfo::kNone) {
    return FunctionIdToScopeId(timer_info.function_id());
  }

  // TODO (b/226565085) remove the flag check when the manual instrumentation grouping feature is
  // released.
  if (absl::GetFlag(FLAGS_devmode) &&
      (timer_info.type() == orbit_client_protos::TimerInfo_Type_kApiScope ||
       timer_info.type() == orbit_client_protos::TimerInfo_Type_kApiScopeAsync)) {
    const auto key = std::make_pair(timer_info.type(), timer_info.api_scope_name());

    const auto it = name_to_id_.find(key);
    if (it != name_to_id_.end()) {
      return it->second;
    }

    uint64_t id = next_id_;
    name_to_id_[key] = id;
    scope_id_to_name_[id] = timer_info.api_scope_name();
    next_id_++;
    return id;
  }
  return orbit_client_data::kInvalidScopeId;
}

[[nodiscard]] std::vector<uint64_t> NameEqualityScopeIdProvider::GetAllProvidedScopeIds() const {
  std::vector<uint64_t> ids;
  std::transform(std::begin(scope_id_to_name_), std::end(scope_id_to_name_),
                 std::back_inserter(ids), [](const auto& entry) { return entry.first; });
  return ids;
}

const std::string& NameEqualityScopeIdProvider::GetScopeName(uint64_t scope_id) const {
  const auto it = scope_id_to_name_.find(scope_id);
  ORBIT_CHECK(it != scope_id_to_name_.end());
  return it->second;
}

}  // namespace orbit_client_data