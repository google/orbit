// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"

#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/ThreadUtils.h"

using orbit_capture_uploader::DataReadiness;
using orbit_grpc_protos::CaptureResponse;
using orbit_grpc_protos::ClientCaptureEvent;

namespace orbit_producer_event_processor {

UploaderClientCaptureEventCollector::UploaderClientCaptureEventCollector() {
  // Create an output stream which will be used to convert capture events into parts of a well
  // formatted capture file. This should never fail.
  output_stream_ =
      orbit_capture_file::CaptureFileOutputStream::Create(&capture_data_buffer_stream_);
}

UploaderClientCaptureEventCollector::~UploaderClientCaptureEventCollector() {
  LOG("Total number of events uploaded: %u", total_uploaded_event_count_);
  LOG("Total number of bytes uploaded: %u", total_uploaded_data_bytes_);

  if (total_uploaded_event_count_ > 0) {
    float average_bytes = static_cast<float>(total_uploaded_data_bytes_) /
                          static_cast<float>(total_uploaded_event_count_);
    LOG("Average number of bytes per event: %.2f", average_bytes);
  }
}

void UploaderClientCaptureEventCollector::Stop() {
  absl::MutexLock lock{&mutex_};

  CHECK(output_stream_ != nullptr);
  auto close_result = output_stream_->Close();
  if (close_result.has_error()) {
    ERROR("Closing output stream: %s", close_result.error().message());
  }
}

void UploaderClientCaptureEventCollector::AddEvent(ClientCaptureEvent&& event) {
  {
    absl::MutexLock lock{&mutex_};

    // The output stream gets closed when processing the `CaptureFinishedEvent`. Drop events
    // received after closing the output stream.
    CHECK(output_stream_ != nullptr);
    if (!output_stream_->IsOpen()) return;

    auto write_result = output_stream_->WriteCaptureEvent(event);
    CHECK(!write_result.has_error());

    ++buffered_event_count_;
    buffered_event_bytes_ += event.ByteSizeLong();
  }

  // Close output stream after processing the `CaptureFinishedEvent`.
  if (event.event_case() == ClientCaptureEvent::kCaptureFinished) Stop();
}

DataReadiness UploaderClientCaptureEventCollector::DetermineDataReadiness() {
  absl::MutexLock lock(&mutex_);

  // Return `kHasData` if not yet finish uploading data in `capture_data_to_upload_`.
  if (byte_position_ < capture_data_to_upload_.size()) {
    return DataReadiness::kHasData;
  }

  // Now there are two possible cases that make `byte_position_ == capture_data_to_upload_.size()`:
  // - case 1: `capture_data_to_upload_` is not empty and we just finished uploading data from it
  // - case 2: `capture_data_to_upload_` is empty as we didn't get new data from the last call of
  //           `capture_data_buffer_stream_.TakeBuffer()`.
  // Update uploaded data if it is case 1. (Nothing happens for case 2.)
  total_uploaded_event_count_ += buffered_event_count_;
  total_uploaded_data_bytes_ += capture_data_to_upload_.size();

  // Refill `capture_data_to_upload_` with data buffered in `capture_data_buffer_stream_`.
  capture_data_to_upload_ = capture_data_buffer_stream_.TakeBuffer();
  byte_position_ = 0;

  // As buffered data is taken away from `capture_data_buffer_stream_`, update the statistics of
  // buffered data.
  if (buffered_event_count_ > 0) {
    ORBIT_UINT64("Number of CaptureEvents to upload", buffered_event_count_);
    ORBIT_UINT64("Bytes of CaptureEvents to upload", buffered_event_bytes_);

    [[maybe_unused]] const float average_bytes =
        static_cast<float>(buffered_event_bytes_) / static_cast<float>(buffered_event_count_);
    ORBIT_FLOAT("Average bytes per CaptureEvent", average_bytes);
  }
  buffered_event_count_ = 0;
  buffered_event_bytes_ = 0;

  // Check again whether there is new data ready.
  if (!capture_data_to_upload_.empty()) {
    return DataReadiness::kHasData;
  }

  // If no new data is filled into `capture_data_to_upload_` and the capture is not finished, return
  // `kWaitingForData`. Note that `output_stream_` will be closed immediately after processing the
  // CaptureFinishedEvent, and all the buffered data in `output_stream_` will be flushed to
  // `capture_data_buffer_stream_`. And this last piece of data should already be taken away by
  // previous call of `capture_data_buffer_stream_.TakeBuffer()` when we find `output_stream_` is
  // closed here.
  if (output_stream_->IsOpen()) {
    return DataReadiness::kWaitingForData;
  }

  return DataReadiness::kEndOfData;
}

size_t UploaderClientCaptureEventCollector::ReadIntoBuffer(void* dest, size_t max_bytes) {
  absl::MutexLock lock(&mutex_);

  size_t bytes_to_read = std::min(capture_data_to_upload_.size() - byte_position_, max_bytes);

  char* dest_buffer = static_cast<char*>(dest);
  std::memcpy(dest_buffer, capture_data_to_upload_.data() + byte_position_, bytes_to_read);
  byte_position_ += bytes_to_read;

  return bytes_to_read;
}

}  // namespace orbit_producer_event_processor