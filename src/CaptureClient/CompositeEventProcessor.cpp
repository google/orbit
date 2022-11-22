// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <utility>
#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {

namespace {

class CompositeEventProcessor : public CaptureEventProcessor {
 public:
  explicit CompositeEventProcessor(
      std::vector<std::unique_ptr<CaptureEventProcessor>> event_processors)
      : event_processors_{std::move(event_processors)} {}

  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) override {
    for (auto& event_processor : event_processors_) {
      event_processor->ProcessEvent(event);
    }
  }

 private:
  std::vector<std::unique_ptr<CaptureEventProcessor>> event_processors_;
};

}  // namespace

std::unique_ptr<CaptureEventProcessor> CaptureEventProcessor::CreateCompositeProcessor(
    std::vector<std::unique_ptr<CaptureEventProcessor>> event_processors) {
  return std::make_unique<CompositeEventProcessor>(std::move(event_processors));
}

}  // namespace orbit_capture_client
