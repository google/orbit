// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_API_EVENT_ID_PROVIDER_H_
#define ORBIT_GL_API_EVENT_ID_PROVIDER_H_

#include <cstdint>

#include "ClientData/TimerTrackDataIdManager.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_gl {

// The inferface defines a map from `TimerInfo` to ids. When called twice on identical `TimerInfo`
// instances instances, it returns the same ids.
class ApiEventIdProvider {
 public:
  virtual ~ApiEventIdProvider() = default;

  [[nodiscard]] virtual uint64_t ProvideId(const TimerInfo& timer_info) = 0;
};

// Provides equal ids to the instances of `TimerInfo` if and only if their
// `api_scope_name` and their `type` are equal.  The ids are chosen consecutively starting with
// `start_id`. To instantiate, use `NameEqualityApiEventIdSetter::Create` as this ensures no
// overlap between `api_scope_group_id` and `function_id`.
class NameEqualityApiEventIdProvider : public ApiEventIdProvider {
 public:
  // Ids for instrumented functions are precomputed on capture start and we are using id range above
  // those.
  [[nodiscard]] static std::unique_ptr<NameEqualityApiEventIdProvider> Create(
      const ::orbit_grpc_protos::CaptureOptions& capture_options);

  [[nodiscard]] uint64_t ProvideId(const TimerInfo& timer_info) override;

 private:
  explicit NameEqualityApiEventIdProvider(uint64_t start_id);

  absl::flat_hash_map<std::pair<orbit_client_protos::TimerInfo_Type, std::string>, uint64_t>
      name_to_id_;
  uint64_t next_id_{};
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_API_EVENT_ID_PROVIDER_H_
