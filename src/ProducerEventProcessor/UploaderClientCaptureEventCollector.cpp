// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"
#include "services.grpc.pb.h"

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

DataReadiness UploaderClientCaptureEventCollector::GetDataReadiness() const {
  absl::MutexLock lock(&mutex_);

  if (!capture_data_to_upload_.empty()) {
    return DataReadiness::kHasData;
  }

  if (output_stream_->IsOpen() || buffered_event_bytes_ > 0) {
    return DataReadiness::kWaitingForData;
  }

  return DataReadiness::kEndOfData;
}

void UploaderClientCaptureEventCollector::RefreshUploadDataBuffer() {
  // Clear `capture_data_to_upload_` immediately.
  capture_data_to_upload_.clear();

  // Refill `capture_data_to_upload_` when there is enough data to upload.
  mutex_.LockWhen(absl::Condition(
      +[](UploaderClientCaptureEventCollector* self) ABSL_EXCLUSIVE_LOCKS_REQUIRED(self->mutex_) {
        constexpr int kUploadEventCountInterval = 5000;
        return self->buffered_event_count_ >= kUploadEventCountInterval ||
               !self->output_stream_->IsOpen();
      },
      this));

  ORBIT_UINT64("Number of CaptureEvents to upload", buffered_event_count_);
  ORBIT_UINT64("Bytes of CaptureEvents to upload", buffered_event_bytes_);
  if (buffered_event_count_ > 0) {
    [[maybe_unused]] float average_bytes =
        static_cast<float>(buffered_event_bytes_) / static_cast<float>(buffered_event_count_);
    ORBIT_FLOAT("Average bytes per CaptureEvent", average_bytes);
  }

  capture_data_to_upload_ = capture_data_buffer_stream_.TakeBuffer();
  if (!capture_data_to_upload_.empty()) {
    total_uploaded_event_count_ += buffered_event_count_;
    total_uploaded_data_bytes_ += capture_data_to_upload_.size();
    buffered_event_count_ = 0;
    buffered_event_bytes_ = 0;
  }
  mutex_.Unlock();
}

}  // namespace orbit_producer_event_processor