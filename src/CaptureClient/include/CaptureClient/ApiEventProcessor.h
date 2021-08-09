// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_
#define CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>

#include "Api/EncodedEvent.h"
#include "CaptureClient/CaptureListener.h"
#include "capture.pb.h"

namespace orbit_capture_client {

// The ApiEventProcessor is responsible for processing orbit_grpc_protos::ApiEvent events and
// transforming them into TimerInfo objects that are relayed to a CaptureListener. Internal state
// is maintained to cache "start" events until a corresponding "stop" event is received. The pair
// is then used to create a single TimerInfo object. "Tracking" events don't need to be cached
// however, they are translated to TimerInfo objects that are directly passed to the listener.
class ApiEventProcessor {
 public:
  explicit ApiEventProcessor(CaptureListener* listener);
  // The new manual instrumentation events (see below) could not use `ApiEvent`, so this is
  // deprecated. The methods for the concrete (new) events should be used instead.
  [[deprecated]] void ProcessApiEventLegacy(const orbit_grpc_protos::ApiEvent& event_buffer);
  void ProcessApiScopeStart(const orbit_grpc_protos::ApiScopeStart& event_buffer);
  void ProcessApiScopeStartAsync(const orbit_grpc_protos::ApiScopeStartAsync& event_buffer);
  void ProcessApiScopeStop(const orbit_grpc_protos::ApiScopeStop& event_buffer);
  void ProcessApiScopeStopAsync(const orbit_grpc_protos::ApiScopeStopAsync& event_buffer);
  void ProcessApiStringEvent(const orbit_grpc_protos::ApiStringEvent& event_buffer);
  void ProcessApiTrackDouble(const orbit_grpc_protos::ApiTrackDouble& event_buffer);
  void ProcessApiTrackFloat(const orbit_grpc_protos::ApiTrackFloat& event_buffer);
  void ProcessApiTrackInt(const orbit_grpc_protos::ApiTrackInt& event_buffer);
  void ProcessApiTrackInt64(const orbit_grpc_protos::ApiTrackInt64& event_buffer);
  void ProcessApiTrackUint(const orbit_grpc_protos::ApiTrackUint& event_buffer);
  void ProcessApiTrackUint64(const orbit_grpc_protos::ApiTrackUint64& event_buffer);

 private:
  [[deprecated]] void ProcessApiEventLegacy(const orbit_api::ApiEvent& api_event);
  [[deprecated]] void ProcessStartEventLegacy(const orbit_api::ApiEvent& api_event);
  [[deprecated]] void ProcessStopEventLegacy(const orbit_api::ApiEvent& api_event);
  [[deprecated]] void ProcessAsyncStartEventLegacy(const orbit_api::ApiEvent& api_event);
  [[deprecated]] void ProcessAsyncStopEventLegacy(const orbit_api::ApiEvent& api_event);
  [[deprecated]] void ProcessTrackingEventLegacy(const orbit_api::ApiEvent& api_event);
  [[deprecated]] void ProcessStringEventLegacy(const orbit_api::ApiEvent& api_event);

  CaptureListener* capture_listener_ = nullptr;
  absl::flat_hash_map<int32_t, std::vector<orbit_api::ApiEvent>> synchronous_event_stack_by_tid_;
  absl::flat_hash_map<int32_t, std::vector<orbit_grpc_protos::ApiScopeStart>>
      synchronous_scopes_stack_by_tid_;
  absl::flat_hash_map<int32_t, orbit_api::ApiEvent> asynchronous_events_by_id_;
  absl::flat_hash_map<int32_t, orbit_grpc_protos::ApiScopeStartAsync> asynchronous_scopes_by_id_;
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_
