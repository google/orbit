// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CLIENT_DATA_API_SCOPE_ID_PROVIDER_H_
#define ORBIT_CLIENT_DATA_API_SCOPE_ID_PROVIDER_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/synchronization/mutex.h>

#include <cstdint>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_client_data {

// The interface defines a map from `TimerInfo` to ScopeId. When called twice on identical
// `TimerInfo` instances, it returns the same ScopeId.
class ScopeIdProvider {
 public:
  virtual ~ScopeIdProvider() = default;

  [[nodiscard]] virtual std::optional<ScopeId> FunctionIdToScopeId(uint64_t function_id) const = 0;
  [[nodiscard]] virtual uint64_t ScopeIdToFunctionId(ScopeId scope_id) const = 0;

  // TODO(b/243122633) The method is added in the hope to investigate the crash via additional
  // runtime checks. When the bug is resolved, the method should be removed.
  [[nodiscard]] virtual ScopeId GetMaxId() const = 0;

  [[nodiscard]] virtual std::optional<ScopeId> ProvideId(const TimerInfo& timer_info) = 0;

  [[nodiscard]] virtual std::vector<ScopeId> GetAllProvidedScopeIds() const = 0;

  [[nodiscard]] virtual const ScopeInfo& GetScopeInfo(ScopeId scope_id) const = 0;

  [[nodiscard]] virtual const FunctionInfo* GetFunctionInfo(ScopeId scope_id) const = 0;

  [[nodiscard]] virtual std::optional<uint64_t> FindFunctionIdSlow(
      const FunctionInfo& function_info) const = 0;

  virtual void UpdateFunctionInfoAddress(
      orbit_grpc_protos::InstrumentedFunction instrumented_function) = 0;
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

  [[nodiscard]] std::optional<ScopeId> FunctionIdToScopeId(uint64_t function_id) const override;
  [[nodiscard]] uint64_t ScopeIdToFunctionId(ScopeId scope_id) const override;

  [[nodiscard]] ScopeId GetMaxId() const override {
    absl::ReaderMutexLock reader_lock{&mutex_};
    return ScopeId(*next_id_ - 1);
  }

  [[nodiscard]] std::optional<ScopeId> ProvideId(const TimerInfo& timer_info) override;

  [[nodiscard]] std::vector<ScopeId> GetAllProvidedScopeIds() const override;

  [[nodiscard]] const ScopeInfo& GetScopeInfo(ScopeId scope_id) const override;

  [[nodiscard]] const FunctionInfo* GetFunctionInfo(ScopeId scope_id) const override;

  [[nodiscard]] std::optional<uint64_t> FindFunctionIdSlow(
      const FunctionInfo& function_info) const override;

  void UpdateFunctionInfoAddress(
      orbit_grpc_protos::InstrumentedFunction instrumented_function) override;

 private:
  explicit NameEqualityScopeIdProvider(
      uint64_t start_id, absl::flat_hash_map<const ScopeInfo, ScopeId> scope_info_to_id,
      absl::flat_hash_map<ScopeId, const ScopeInfo> scope_id_to_info,
      absl::flat_hash_map<ScopeId, FunctionInfo> scope_id_to_function_info)
      : next_id_(ScopeId(start_id)),
        max_instrumented_function_id_(ScopeId(start_id - 1)),
        scope_info_to_id_(std::move(scope_info_to_id)),
        scope_id_to_info_(std::move(scope_id_to_info)),
        scope_id_to_function_info_(std::move(scope_id_to_function_info)) {}

  [[nodiscard]] std::optional<ScopeId> GetExistingScopeId(const ScopeInfo& scope_info) const
      ABSL_SHARED_LOCKS_REQUIRED(mutex_);

  ScopeId next_id_ ABSL_GUARDED_BY(mutex_){};
  ScopeId max_instrumented_function_id_{};
  absl::flat_hash_map<const ScopeInfo, ScopeId> scope_info_to_id_ ABSL_GUARDED_BY(mutex_);
  absl::flat_hash_map<ScopeId, const ScopeInfo> scope_id_to_info_ ABSL_GUARDED_BY(mutex_);
  /// TODO(http://b/247467504): Add FunctionInfo to ScopeInfo.
  absl::flat_hash_map<ScopeId, FunctionInfo> scope_id_to_function_info_;
  mutable absl::Mutex mutex_;
};

}  // namespace orbit_client_data

#endif  // ORBIT_CLIENT_DATA_API_SCOPE_ID_PROVIDER_H_
