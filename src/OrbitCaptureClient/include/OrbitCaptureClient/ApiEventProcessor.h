// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_
#define ORBIT_CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_map.h>

#include "OrbitCaptureClient/CaptureListener.h"
#include "capture.pb.h"

class ApiEventProcessor {
 public:
  ApiEventProcessor(CaptureListener* listener);
  void ProcessApiEvent(const orbit_grpc_protos::ApiEvent& api_event);

 private:
  void ProcessStartEvent(const orbit_grpc_protos::ApiEvent& api_event);
  void ProcessStopEvent(const orbit_grpc_protos::ApiEvent& api_event);
  void ProcessAsyncStartEvent(const orbit_grpc_protos::ApiEvent& api_event);
  void ProcessAsyncStopEvent(const orbit_grpc_protos::ApiEvent& api_event);
  void ProcessTrackingEvent(const orbit_grpc_protos::ApiEvent& api_event);

 private:
  CaptureListener* capture_listener_ = nullptr;
  absl::flat_hash_map<int32_t, std::vector<orbit_grpc_protos::ApiEvent>>
      synchronous_event_stack_by_tid_;
  absl::flat_hash_map<int32_t, orbit_grpc_protos::ApiEvent> asynchronous_events_by_id_;
};

#endif  // ORBIT_CAPTURE_CLIENT_API_EVENT_PROCESSOR_H_
