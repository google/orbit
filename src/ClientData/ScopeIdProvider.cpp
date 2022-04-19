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
#include "ClientData/ScopeInfo.h"
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

  absl::flat_hash_map<uint64_t, const ScopeInfo> scope_id_to_info;
  for (const auto& instrumented_function : instrumented_functions) {
    scope_id_to_info.insert({instrumented_function.function_id(),
                             {instrumented_function.function_name(), ScopeType::kHookedFunction}});
  }

  return std::unique_ptr<NameEqualityScopeIdProvider>(
      new NameEqualityScopeIdProvider(max_id + 1, std::move(scope_id_to_info)));
}

uint64_t NameEqualityScopeIdProvider::FunctionIdToScopeId(uint64_t function_id) const {
  return function_id;
}

[[nodiscard]] static ScopeType ScopeTypeFromTimerInfo(const TimerInfo& timer) {
  switch (timer.type()) {
    case TimerInfo::kNone:
      return timer.function_id() != orbit_grpc_protos::kInvalidFunctionId
                 ? ScopeType::kHookedFunction
                 : ScopeType::kOther;
    case TimerInfo::kApiScope:
      return ScopeType::kApiScope;
    case TimerInfo::kApiScopeAsync:
      return ScopeType::kApiScopeAsync;
    default:
      return ScopeType::kOther;
  }
}

uint64_t NameEqualityScopeIdProvider::ProvideId(const TimerInfo& timer_info) {
  const ScopeType scope_type = ScopeTypeFromTimerInfo(timer_info);

  if (scope_type == ScopeType::kOther) return orbit_client_data::kInvalidScopeId;

  if (scope_type == ScopeType::kHookedFunction) {
    return FunctionIdToScopeId(timer_info.function_id());
  }

  // TODO (b/226565085) remove the flag check when the manual instrumentation grouping feature is
  // released.
  if (!absl::GetFlag(FLAGS_devmode)) return kInvalidScopeId;

  const ScopeInfo scope_info{timer_info.api_scope_name(), scope_type};

  const auto it = scope_info_to_id_.find(scope_info);
  if (it != scope_info_to_id_.end()) {
    return it->second;
  }

  uint64_t id = next_id_;
  scope_info_to_id_[scope_info] = id;
  scope_id_to_info_.insert({id, scope_info});
  next_id_++;
  return id;
}

[[nodiscard]] std::vector<uint64_t> NameEqualityScopeIdProvider::GetAllProvidedScopeIds() const {
  std::vector<uint64_t> ids;
  std::transform(std::begin(scope_id_to_info_), std::end(scope_id_to_info_),
                 std::back_inserter(ids), [](const auto& entry) { return entry.first; });
  return ids;
}

const ScopeInfo& NameEqualityScopeIdProvider::GetScopeInfo(uint64_t scope_id) const {
  const auto it = scope_id_to_info_.find(scope_id);
  ORBIT_CHECK(it != scope_id_to_info_.end());
  return it->second;
}

}  // namespace orbit_client_data