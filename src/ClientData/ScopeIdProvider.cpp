// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/ScopeIdProvider.h"

#include <absl/container/flat_hash_map.h>
#include <absl/meta/type_traits.h>
#include <absl/synchronization/mutex.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"

namespace orbit_client_data {

using orbit_grpc_protos::InstrumentedFunction;

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

  absl::flat_hash_map<ScopeId, const ScopeInfo> scope_id_to_info;
  absl::flat_hash_map<const ScopeInfo, ScopeId> scope_info_to_id;
  absl::flat_hash_map<ScopeId, FunctionInfo> scope_id_to_function_info;

  for (const auto& instrumented_function : instrumented_functions) {
    const ScopeId scope_id{instrumented_function.function_id()};
    const ScopeInfo scope_info(instrumented_function.function_name(),
                               ScopeType::kDynamicallyInstrumentedFunction);
    scope_id_to_info.try_emplace(scope_id, scope_info);
    scope_info_to_id.try_emplace(scope_info, scope_id);
    scope_id_to_function_info.try_emplace(
        scope_id, /* in-place FunctionInfo construction */ instrumented_function.file_path(),
        instrumented_function.file_build_id(), instrumented_function.function_virtual_address(),
        instrumented_function.function_size(), instrumented_function.function_name(),
        instrumented_function.is_hotpatchable());
  }

  return std::unique_ptr<NameEqualityScopeIdProvider>(new NameEqualityScopeIdProvider(
      max_id + 1, std::move(scope_info_to_id), std::move(scope_id_to_info),
      std::move(scope_id_to_function_info)));
}

std::optional<ScopeId> NameEqualityScopeIdProvider::FunctionIdToScopeId(
    uint64_t function_id) const {
  if (function_id == orbit_grpc_protos::kInvalidFunctionId) return std::nullopt;
  return ScopeId(function_id);
}

[[nodiscard]] static ScopeType ScopeTypeFromTimerInfo(const TimerInfo& timer) {
  switch (timer.type()) {
    case TimerInfo::kNone:
      return timer.function_id() != orbit_grpc_protos::kInvalidFunctionId
                 ? ScopeType::kDynamicallyInstrumentedFunction
                 : ScopeType::kInvalid;
    case TimerInfo::kApiScope:
      return ScopeType::kApiScope;
    case TimerInfo::kApiScopeAsync:
      return ScopeType::kApiScopeAsync;
    default:
      return ScopeType::kInvalid;
  }
}

std::optional<ScopeId> NameEqualityScopeIdProvider::ProvideId(const TimerInfo& timer_info) {
  const ScopeType scope_type = ScopeTypeFromTimerInfo(timer_info);

  if (scope_type == ScopeType::kInvalid) return std::nullopt;

  if (scope_type == ScopeType::kDynamicallyInstrumentedFunction) {
    return FunctionIdToScopeId(timer_info.function_id());
  }

  ORBIT_CHECK(scope_type == ScopeType::kApiScope || scope_type == ScopeType::kApiScopeAsync);

  const ScopeInfo scope_info{timer_info.api_scope_name(), scope_type};

  {
    absl::ReaderMutexLock reader_lock{&mutex_};
    if (std::optional<ScopeId> id = GetExistingScopeId(scope_info); id.has_value()) {
      return id.value();
    }
  }

  absl::WriterMutexLock writer_local{&mutex_};

  if (std::optional<ScopeId> id = GetExistingScopeId(scope_info); id.has_value()) {
    return id.value();
  }

  const ScopeId id = next_id_++;

  scope_info_to_id_.emplace(scope_info, id);
  scope_id_to_info_.emplace(id, scope_info);
  return id;
}

[[nodiscard]] std::vector<ScopeId> NameEqualityScopeIdProvider::GetAllProvidedScopeIds() const {
  absl::ReaderMutexLock reader_lock{&mutex_};
  std::vector<ScopeId> ids;
  std::transform(std::begin(scope_id_to_info_), std::end(scope_id_to_info_),
                 std::back_inserter(ids), [](const auto& entry) { return entry.first; });
  return ids;
}

const ScopeInfo& NameEqualityScopeIdProvider::GetScopeInfo(ScopeId scope_id) const {
  absl::ReaderMutexLock reader_lock{&mutex_};
  const auto it = scope_id_to_info_.find(scope_id);
  ORBIT_CHECK(it != scope_id_to_info_.end());
  return it->second;
}

uint64_t NameEqualityScopeIdProvider::ScopeIdToFunctionId(ScopeId scope_id) const {
  if (scope_id <= max_instrumented_function_id_) return *scope_id;
  return orbit_grpc_protos::kInvalidFunctionId;
}

std::optional<ScopeId> NameEqualityScopeIdProvider::GetExistingScopeId(
    const ScopeInfo& scope_info) const {
  if (const auto it = scope_info_to_id_.find(scope_info); it != scope_info_to_id_.end()) {
    return it->second;
  }
  return std::nullopt;
}

const FunctionInfo* NameEqualityScopeIdProvider::GetFunctionInfo(ScopeId scope_id) const {
  const auto it = scope_id_to_function_info_.find(scope_id);
  if (it != scope_id_to_function_info_.end()) {
    return &it->second;
  }
  return nullptr;
}

void NameEqualityScopeIdProvider::UpdateFunctionInfoAddress(
    InstrumentedFunction instrumented_function) {
  if (auto scope_id = FunctionIdToScopeId(instrumented_function.function_id());
      scope_id.has_value()) {
    if (const auto it = scope_id_to_function_info_.find(scope_id.value());
        it != scope_id_to_function_info_.end()) {
      it->second.SetAddress(instrumented_function.function_virtual_address());
    }
  }
}

std::optional<uint64_t> NameEqualityScopeIdProvider::FindFunctionIdSlow(
    const FunctionInfo& function_info) const {
  for (const auto& [scope_id, candidate_function] : scope_id_to_function_info_) {
    if (candidate_function.module_path() == function_info.module_path() &&
        candidate_function.address() == function_info.address()) {
      return ScopeIdToFunctionId(scope_id);
    }
  }
  return std::nullopt;
}

}  // namespace orbit_client_data