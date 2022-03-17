// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_API_EVENT_ID_SETTER_H_
#define CAPTURE_CLIENT_API_EVENT_ID_SETTER_H_

#include <cstdint>

#include "ClientData/TimerTrackDataIdManager.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {
class ApiEventIdSetter {
 public:
  static ApiEventIdSetter Create(const ::orbit_grpc_protos::CaptureOptions& capture_options);

  ApiEventIdSetter() = default;

  void SetId(TimerInfo& timer_info);

 private:
  explicit ApiEventIdSetter(uint64_t start_id);

  absl::flat_hash_map<std::string, uint64_t> name_to_id_;
  uint64_t next_id_{};
};
}  // namespace orbit_capture_client

#endif /* CAPTURE_CLIENT_API_EVENT_ID_SETTER_H_ */
