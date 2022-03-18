// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_API_EVENT_ID_PROVIDER_H_
#define CAPTURE_CLIENT_API_EVENT_ID_PROVIDER_H_

#include <cstdint>

#include "ClientData/TimerTrackDataIdManager.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {

// The inferface defines a map from timer_info to ids.
// When called twice of identical `timer_info` instances,
// it should return the same ids.
class ApiEventIdProvider {
 public:
  virtual ~ApiEventIdProvider() = default;

  [[nodiscard]] virtual uint64_t ProvideId(const TimerInfo& timer_info) = 0;
};

// Sets equal `api_scope_group_id` to the instances of TimeInfo
// if and only if their `api_scope_name` are equal.
// The ids are chosen consequtively starting with `start_id`.
//
// To instantiate, use `NameEqualityApiEventIdSetter::Create`
// as this ensures no overlap between `api_scope_group_id`
// and `function_id`.
class NameEqualityApiEventIdProvider : public ApiEventIdProvider {
 public:
  // Ids for instrumented functions are precomputed on capture start and we are using id
  // range above those.
  [[nodiscard]] static std::unique_ptr<NameEqualityApiEventIdProvider> Create(
      const ::orbit_grpc_protos::CaptureOptions& capture_options);

  NameEqualityApiEventIdProvider() = default;
  explicit NameEqualityApiEventIdProvider(uint64_t start_id);

  [[nodiscard]] uint64_t ProvideId(const TimerInfo& timer_info) override;

 private:
  absl::flat_hash_map<std::string, uint64_t> name_to_id_;
  uint64_t next_id_{};
};

}  // namespace orbit_capture_client

#endif /* CAPTURE_CLIENT_API_EVENT_ID_PROVIDER_H_ */
