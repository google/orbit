// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_API_SCOPE_ID_PROVIDER_H_
#define ORBIT_CLIENT_DATA_API_SCOPE_ID_PROVIDER_H_

#include <absl/container/flat_hash_map.h>

#include <cstdint>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

// The interface defines a map from `TimerInfo` to ids. When called twice on identical `TimerInfo`
// instances, it returns the same ids. This is used to allow for aggregation of manual
// instrumentation events in the same function as we aggregate the events for hooked functions (see
// go/stadia-orbit-manual-instrumentation-aggregation).
class ScopeIdProvider {
 public:
  virtual ~ScopeIdProvider() = default;

  [[nodiscard]] virtual ScopeId FunctionIdToScopeId(uint64_t function_id) const = 0;
  [[nodiscard]] virtual uint64_t ScopeIdToFunctionId(ScopeId scope_id) const = 0;

  [[nodiscard]] virtual ScopeId ProvideId(const TimerInfo& timer_info) = 0;

  [[nodiscard]] virtual std::vector<ScopeId> GetAllProvidedScopeIds() const = 0;

  [[nodiscard]] virtual const ScopeInfo& GetScopeInfo(ScopeId scope_id) const = 0;
};

// This class, unless the timer does already have an id (`function_id`), it assigning an id for
// timers based on the equality of their names and types. Two timers are provided with the same ids
// if and only if they share the name and the type. Currently this is only implemented for manual
// instrumentation scopes, both sync and async.
class NameEqualityScopeIdProvider : public ScopeIdProvider {
 public:
  // Ids for instrumented functions are precomputed on capture start and we are using id range above
  // those.
  [[nodiscard]] static std::unique_ptr<NameEqualityScopeIdProvider> Create(
      const ::orbit_grpc_protos::CaptureOptions& capture_options);

  [[nodiscard]] ScopeId FunctionIdToScopeId(uint64_t function_id) const override;
  [[nodiscard]] uint64_t ScopeIdToFunctionId(ScopeId scope_id) const override;

  [[nodiscard]] ScopeId ProvideId(const TimerInfo& timer_info) override;

  [[nodiscard]] std::vector<ScopeId> GetAllProvidedScopeIds() const override;

  [[nodiscard]] const ScopeInfo& GetScopeInfo(ScopeId scope_id) const override;

 private:
  explicit NameEqualityScopeIdProvider(
      uint64_t start_id, absl::flat_hash_map<const ScopeInfo, ScopeId> scope_info_to_id,
      absl::flat_hash_map<ScopeId, const ScopeInfo> scope_id_to_info)
      : next_id_(start_id),
        max_instrumented_function_id(start_id - 1),
        scope_info_to_id_(std::move(scope_info_to_id)),
        scope_id_to_info_(std::move(scope_id_to_info)) {}

  uint64_t next_id_{};
  uint64_t max_instrumented_function_id{};
  absl::flat_hash_map<const ScopeInfo, ScopeId> scope_info_to_id_;
  absl::flat_hash_map<ScopeId, const ScopeInfo> scope_id_to_info_;
};

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_API_SCOPE_ID_PROVIDER_H_
