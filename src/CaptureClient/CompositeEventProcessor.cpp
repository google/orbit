// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/CaptureEventProcessor.h"

namespace orbit_capture_client {

namespace {

class CompositeEventProcessor : public CaptureEventProcessor {
 public:
  explicit CompositeEventProcessor(absl::Span<CaptureEventProcessor* const> event_processors)
      : event_processors_(event_processors.begin(), event_processors.end()) {}

  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) override {
    for (auto& event_processor : event_processors_) {
      event_processor->ProcessEvent(event);
    }
  }

 private:
  std::vector<CaptureEventProcessor*> event_processors_;
};

}  // namespace

std::unique_ptr<CaptureEventProcessor> CaptureEventProcessor::CreateCompositeProcessor(
    absl::Span<CaptureEventProcessor* const> event_processors) {
  return std::make_unique<CompositeEventProcessor>(event_processors);
}

}  // namespace orbit_capture_client
