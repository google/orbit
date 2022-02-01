// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_
#define CAPTURE_SERVICE_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_

#include "ProducerEventProcessor/ClientCaptureEventCollector.h"

namespace orbit_capture_service {

class ClientCaptureEventCollectorBuilder {
 public:
  virtual ~ClientCaptureEventCollectorBuilder() = default;
  virtual std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
  BuildClientCaptureEventCollector() = 0;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_