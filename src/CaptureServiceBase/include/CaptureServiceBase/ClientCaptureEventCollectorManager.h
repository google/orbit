// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_
#define CAPTURE_SERVICE_BASE_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_

#include "ProducerEventProcessor/ClientCaptureEventCollector.h"

namespace orbit_capture_service_base {

// This is a gRPC-free interface for creating and managing `ClientCaptureEventCollector`. Native
// orbit capture services and the cloud collector will have their own implementations to build
// either a `GrpcClientCaptureEventCollector` or a `UploaderClientCaptureEventCollector`.
class ClientCaptureEventCollectorManager {
 public:
  virtual ~ClientCaptureEventCollectorManager() = default;
  [[nodiscard]] virtual orbit_producer_event_processor::ClientCaptureEventCollector*
  GetClientCaptureEventCollector() = 0;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_