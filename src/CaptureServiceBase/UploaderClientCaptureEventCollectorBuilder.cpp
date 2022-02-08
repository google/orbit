// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/UploaderClientCaptureEventCollectorBuilder.h"

#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"

namespace orbit_capture_service_base {

// A `ClientCaptureEventCollectorBuilder` implementation to build
// `UploaderClientCaptureEventCollector` for the cloud collector.
class UploaderClientCaptureEventCollectorBuilder : public ClientCaptureEventCollectorBuilder {
 public:
  std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
  BuildClientCaptureEventCollector() override {
    return std::make_unique<orbit_producer_event_processor::UploaderClientCaptureEventCollector>();
  }
};

std::unique_ptr<ClientCaptureEventCollectorBuilder>
CreateUploaderClientCaptureEventCollectorBuilder() {
  return std::make_unique<UploaderClientCaptureEventCollectorBuilder>();
}

}  // namespace orbit_capture_service_base