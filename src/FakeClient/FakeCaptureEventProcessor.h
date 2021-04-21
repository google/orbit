// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_CLIENT_FAKE_CAPTURE_EVENT_PROCESSOR_H_
#define FAKE_CLIENT_FAKE_CAPTURE_EVENT_PROCESSOR_H_

#include "OrbitBase/WriteStringToFile.h"
#include "OrbitCaptureClient/CaptureEventProcessor.h"

namespace orbit_fake_client {

// This implementation of CaptureEventProcessor simply discards all the events it receives, but it
// keeps track of their number and total size and then outputs these statistics to file.
class FakeCaptureEventProcessor : public CaptureEventProcessor {
 public:
  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) override {
    ++event_count_;
    byte_count_ += event.ByteSizeLong();
  }

  ~FakeCaptureEventProcessor() override {
    {
      LOG("Events received: %u", event_count_);
      auto event_count_write_result =
          orbit_base::WriteStringToFile(kEventCountFilename, std::to_string(event_count_));
      if (event_count_write_result.has_error()) {
        FATAL("Writing to \"%s\": %s", kEventCountFilename,
              event_count_write_result.error().message());
      }
    }

    {
      LOG("Bytes received: %u", byte_count_);
      auto byte_count_write_result =
          orbit_base::WriteStringToFile(kByteCountFilename, std::to_string(byte_count_));
      if (byte_count_write_result.has_error()) {
        FATAL("Writing to \"%s\": %s", kByteCountFilename,
              byte_count_write_result.error().message());
      }
    }
  }

 private:
  static constexpr const char* kEventCountFilename = "OrbitFakeClient.event_count.txt";
  static constexpr const char* kByteCountFilename = "OrbitFakeClient.byte_count.txt";

  uint64_t event_count_ = 0;
  uint64_t byte_count_ = 0;
};

}  // namespace orbit_fake_client

#endif  // FAKE_CLIENT_FAKE_CAPTURE_EVENT_PROCESSOR_H_
