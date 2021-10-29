// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ProducerEventProcessor/UploaderCaptureEventCollector.h"

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

UploaderCaptureEventCollector::UploaderCaptureEventCollector() {
  // Create an output stream which will be used to convert capture events into parts of a well
  // formatted capture file. This should never fail.
  capture_data_buffer_stream_ = std::make_unique<orbit_capture_file::BufferOutputStream>();
  output_stream_ =
      orbit_capture_file::CaptureFileOutputStream::Create(capture_data_buffer_stream_.get());
}

UploaderCaptureEventCollector::~UploaderCaptureEventCollector() {
  LOG("Total number of events uploaded: %u", total_uploaded_event_count_);
  LOG("Total number of bytes uploaded: %u", total_uploaded_data_bytes_);
  LOG("Total number of write event errors: %u", total_write_error_count_);

  if (total_uploaded_event_count_ > 0) {
    float average_bytes = static_cast<float>(total_uploaded_data_bytes_) /
                          static_cast<float>(total_uploaded_event_count_);
    LOG("Average number of bytes per event: %.2f", average_bytes);
  }
}

void UploaderCaptureEventCollector::Stop() {
  absl::MutexLock lock{&mutex_};

  // Protect stop_requested_ with mutex_ so that we can use stop_requested_ in Conditions for
  // Await/LockWhen.
  stop_requested_ = true;

  // Capture finish event would be processed before requesting stopping this collector, and the
  // output stream should already be closed while processing the capture finish event.
  CHECK(!output_stream_->IsOpen());
}

void UploaderCaptureEventCollector::AddEvent(ClientCaptureEvent&& event) {
  absl::MutexLock lock{&mutex_};

  // Drop events received after "stop capture" being requested.
  if (stop_requested_) return;

  // The output stream will be closed while processing the capture finish event. Drop events
  // received after closing the output stream.
  CHECK(output_stream_ != nullptr);
  if (!output_stream_->IsOpen()) return;

  auto write_result = output_stream_->WriteCaptureEvent(event);
  if (write_result.has_error()) {
    total_write_error_count_++;
    return;
  }
  buffered_event_count_++;
  buffered_event_bytes_ += event.ByteSizeLong();

  // Close output stream after processing the capture finish event.
  if (event.event_case() != ClientCaptureEvent::kCaptureFinished) return;
  auto close_result = output_stream_->Close();
  if (close_result.has_error()) {
    LOG("Error while closing output steam: %s", close_result.error().message());
  }
}

DataReadiness UploaderCaptureEventCollector::GetDataReadiness() const {
  absl::MutexLock lock(&mutex_);

  if (!capture_data_to_upload_.empty()) {
    return DataReadiness::kHasData;

  } else if (!stop_requested_ || buffered_event_bytes_ > 0) {
    return DataReadiness::kWaitingForData;

  } else {
    return DataReadiness::kEndOfData;
  }
}

void UploaderCaptureEventCollector::RefreshUploadDataBuffer() {
  // Clear capture_data_to_upload_ immediately.
  capture_data_to_upload_.clear();

  // Refill capture_data_to_upload_ when there is enough data to upload or we have been waiting
  // for enough time.
  constexpr int kUploadEventCountInterval = 5000;
  constexpr absl::Duration kWaitDuration = absl::Milliseconds(20);
  mutex_.LockWhenWithTimeout(
      absl::Condition(
          +[](UploaderCaptureEventCollector* self) ABSL_EXCLUSIVE_LOCKS_REQUIRED(self->mutex_) {
            return self->buffered_event_count_ >= kUploadEventCountInterval ||
                   self->stop_requested_;
          },
          this),
      kWaitDuration);

  ORBIT_INT("Number of CaptureEvents to upload", buffered_event_count_);
  ORBIT_INT("Bytes of CaptureEvents to upload", buffered_event_bytes_);
  if (buffered_event_count_ > 0) {
    float average_bytes =
        static_cast<float>(buffered_event_bytes_) / static_cast<float>(buffered_event_count_);
    ORBIT_FLOAT("Average bytes per CaptureEvent", average_bytes);
  }
  total_uploaded_event_count_ += buffered_event_count_;
  total_uploaded_data_bytes_ += capture_data_to_upload_.size();

  capture_data_to_upload_ = capture_data_buffer_stream_->TakeBuffer();
  buffered_event_count_ = 0;
  buffered_event_bytes_ = 0;
  mutex_.Unlock();
}

}  // namespace orbit_producer_event_processor