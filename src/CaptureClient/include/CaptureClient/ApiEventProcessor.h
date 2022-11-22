// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_
#define CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>

#include <cstdint>
#include <vector>

#include "CaptureClient/CaptureListener.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {

// The ApiEventProcessor is responsible for processing all the orbit_grpc_protos::Api... events and
// transforming them into TimerInfo objects that are relayed to a CaptureListener. Internal state
// is maintained to cache "start" events until a corresponding "stop" event is received. The pair
// is then used to create a single TimerInfo object. "Tracking" events don't need to be cached
// however, they are translated to TimerInfo objects that are directly passed to the listener.
class ApiEventProcessor {
 public:
  explicit ApiEventProcessor(CaptureListener* listener);

  void ProcessApiScopeStart(const orbit_grpc_protos::ApiScopeStart& api_scope_start);
  void ProcessApiScopeStartAsync(
      const orbit_grpc_protos::ApiScopeStartAsync& grpc_api_scope_start_async);
  void ProcessApiScopeStop(const orbit_grpc_protos::ApiScopeStop& grpc_api_scope_stop);
  void ProcessApiScopeStopAsync(
      const orbit_grpc_protos::ApiScopeStopAsync& grpc_api_scope_stop_async);
  void ProcessApiStringEvent(const orbit_grpc_protos::ApiStringEvent& grpc_api_string_event);
  void ProcessApiTrackDouble(const orbit_grpc_protos::ApiTrackDouble& grpc_api_track_double);
  void ProcessApiTrackFloat(const orbit_grpc_protos::ApiTrackFloat& grpc_api_track_float);
  void ProcessApiTrackInt(const orbit_grpc_protos::ApiTrackInt& grpc_api_track_int);
  void ProcessApiTrackInt64(const orbit_grpc_protos::ApiTrackInt64& grpc_api_track_int64);
  void ProcessApiTrackUint(const orbit_grpc_protos::ApiTrackUint& grpc_api_track_uint);
  void ProcessApiTrackUint64(const orbit_grpc_protos::ApiTrackUint64& grpc_api_track_uint64);

 private:
  CaptureListener* capture_listener_ = nullptr;
  absl::flat_hash_map<int32_t, std::vector<orbit_grpc_protos::ApiScopeStart>>
      synchronous_scopes_stack_by_tid_;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::ApiScopeStartAsync> asynchronous_scopes_by_id_;
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_
