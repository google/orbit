// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "CaptureClient/CaptureEventProcessor.h"
#include "GrpcProtos/capture.pb.h"

namespace orbit_capture_client {

class MockEventProcessor : public CaptureEventProcessor {
 public:
  MOCK_METHOD(void, ProcessEvent, (const orbit_grpc_protos::ClientCaptureEvent& event), (override));
};

TEST(CompositeEventProcessor, Smoke) {
  // CompositeEventProcessor takes ownership of event processors used to construct it.
  // But in the test we want to keep pointers to them to check that they are called.
  std::vector<std::unique_ptr<CaptureEventProcessor>> event_processors;

  auto processor = std::make_unique<MockEventProcessor>();
  MockEventProcessor* processor1 = processor.get();
  event_processors.push_back(std::move(processor));

  processor = std::make_unique<MockEventProcessor>();
  MockEventProcessor* processor2 = processor.get();
  event_processors.push_back(std::move(processor));

  processor = std::make_unique<MockEventProcessor>();
  MockEventProcessor* processor3 = processor.get();
  event_processors.push_back(std::move(processor));

  auto composite_processor =
      CaptureEventProcessor::CreateCompositeProcessor(std::move(event_processors));

  EXPECT_CALL(*processor1, ProcessEvent).Times(1);
  EXPECT_CALL(*processor2, ProcessEvent).Times(1);
  EXPECT_CALL(*processor3, ProcessEvent).Times(1);

  orbit_grpc_protos::ClientCaptureEvent event;
  composite_processor->ProcessEvent(event);
}

}  // namespace orbit_capture_client