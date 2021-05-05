// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "CaptureClient/CaptureEventProcessor.h"

namespace orbit_capture_client {

class MockEventProcessor : public CaptureEventProcessor {
 public:
  MOCK_METHOD(void, ProcessEvent, (const orbit_grpc_protos::ClientCaptureEvent& event), (override));
};

TEST(CompositeEventProcessor, Smoke) {
  MockEventProcessor processor1;
  MockEventProcessor processor2;
  MockEventProcessor processor3;

  auto composite_processor =
      CaptureEventProcessor::CreateCompositeProcessor({&processor1, &processor2, &processor3});

  EXPECT_CALL(processor1, ProcessEvent).Times(1);
  EXPECT_CALL(processor2, ProcessEvent).Times(1);
  EXPECT_CALL(processor3, ProcessEvent).Times(1);

  orbit_grpc_protos::ClientCaptureEvent event;
  composite_processor->ProcessEvent(event);
}

}  // namespace orbit_capture_client