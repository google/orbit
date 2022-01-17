// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FAKE_CLIENT_FAKE_CAPTURE_EVENT_PROCESSOR_H_
#define FAKE_CLIENT_FAKE_CAPTURE_EVENT_PROCESSOR_H_

#include "CaptureClient/CaptureEventProcessor.h"
#include "Flags.h"
#include "OrbitBase/WriteStringToFile.h"

namespace orbit_fake_client {

// This implementation of CaptureEventProcessor mostly discard all events it receives, except for:
// - keeping track of their number and total size, and writing these statistics to file;
// - keeping track of the calls to the frame boundary function, and possibly writing the average
//   frame time to file;
class FakeCaptureEventProcessor : public orbit_capture_client::CaptureEventProcessor {
 public:
  void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) override {
    ++event_count_;
    byte_count_ += event.ByteSizeLong();

    // Keep track of the number of calls to the frame boundary function, of the timestamp of the
    // first call, and of the timestamp of the last call. Below, the average frame time is then
    // naively computed as (max_timestamp - min_timestamp) / (call_count - 1).
    if (!event.has_function_call()) {
      return;
    }
    const orbit_grpc_protos::FunctionCall& function_call = event.function_call();
    if (function_call.function_id() != kFrameBoundaryFunctionId) {
      return;
    }
    ++frame_boundary_count_;
    uint64_t start_timestamp_ns = function_call.end_timestamp_ns() - function_call.duration_ns();
    frame_boundary_min_timestamp_ns_ =
        std::min(frame_boundary_min_timestamp_ns_, start_timestamp_ns);
    frame_boundary_max_timestamp_ns_ =
        std::max(frame_boundary_max_timestamp_ns_, start_timestamp_ns);
  }

  ~FakeCaptureEventProcessor() override {
    std::filesystem::path file_path(absl::GetFlag(FLAGS_output_path));
    {
      ORBIT_LOG("Events received: %u", event_count_);
      ErrorMessageOr<void> event_count_write_result = orbit_base::WriteStringToFile(
          file_path / kEventCountFilename, std::to_string(event_count_));
      ORBIT_FAIL_IF(event_count_write_result.has_error(), "Writing to \"%s\": %s",
                    kEventCountFilename, event_count_write_result.error().message());
    }

    {
      ORBIT_LOG("Bytes received: %u", byte_count_);
      ErrorMessageOr<void> byte_count_write_result = orbit_base::WriteStringToFile(
          file_path / kByteCountFilename, std::to_string(byte_count_));
      ORBIT_FAIL_IF(byte_count_write_result.has_error(), "Writing to \"%s\": %s",
                    kByteCountFilename, byte_count_write_result.error().message());
    }

    {
      // If the average frame time is not available, just output an empty string to the file.
      std::string frame_time_ms_string;
      if (frame_boundary_count_ >= 2) {
        double frame_time_ms = static_cast<double>(frame_boundary_max_timestamp_ns_ -
                                                   frame_boundary_min_timestamp_ns_) /
                               1'000'000.0 / static_cast<double>(frame_boundary_count_ - 1);
        frame_time_ms_string = absl::StrFormat("%.3f", frame_time_ms);
        ORBIT_LOG("Avg. frame time (ms): %s", frame_time_ms_string);
      }
      ErrorMessageOr<void> frame_time_write_result =
          orbit_base::WriteStringToFile(file_path / kFrameTimeFilename, frame_time_ms_string);
      ORBIT_FAIL_IF(frame_time_write_result.has_error(), "Writing to \"%s\": %s",
                    kFrameTimeFilename, frame_time_write_result.error().message());
    }
  }

  // Instrument a function with this function id in order for FakeCaptureEventProcessor to use it as
  // frame boundary to compute the average CPU frame time.
  static constexpr uint64_t kFrameBoundaryFunctionId = std::numeric_limits<uint64_t>::max();

 private:
  static constexpr const char* kEventCountFilename = "OrbitFakeClient.event_count.txt";
  static constexpr const char* kByteCountFilename = "OrbitFakeClient.byte_count.txt";
  static constexpr const char* kFrameTimeFilename = "OrbitFakeClient.frame_time.txt";

  uint64_t event_count_ = 0;
  uint64_t byte_count_ = 0;

  uint64_t frame_boundary_count_ = 0;
  uint64_t frame_boundary_min_timestamp_ns_ = std::numeric_limits<uint64_t>::max();
  uint64_t frame_boundary_max_timestamp_ns_ = std::numeric_limits<uint64_t>::min();
};

}  // namespace orbit_fake_client

#endif  // FAKE_CLIENT_FAKE_CAPTURE_EVENT_PROCESSOR_H_