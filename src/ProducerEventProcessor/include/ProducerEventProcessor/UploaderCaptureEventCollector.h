// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_EVENT_PROCESSOR_UPLOADER_CAPTURE_EVENT_COLLECTOR_H_
#define CAPTURE_EVENT_PROCESSOR_UPLOADER_CAPTURE_EVENT_COLLECTOR_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include <memory>
#include <vector>

#include "CaptureFile/CaptureFileOutputStream.h"
#include "CaptureUploader/UploadDataInterface.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"
#include "capture.pb.h"
#include "services.grpc.pb.h"

namespace orbit_producer_event_processor {

// This class receives the ClientCaptureEvents emitted by a ProducerEventProcessor, buffers them
// as a part of a well formated capture file, and enables a CaptureUploader to streaming upload the
// buffered capture data.
// Note that this collector will stop automatically after receiving a CaptureFinish event from the
// ProducerEventProcessor.
class UploaderCaptureEventCollector final : public ClientCaptureEventCollector,
                                            public orbit_capture_uploader::UploadDataInterface {
 public:
  explicit UploaderCaptureEventCollector();
  ~UploaderCaptureEventCollector() override;

  // Convert the received capture event into raw bytes according to the format of capture file, and
  // then buffer the converted data.
  void AddEvent(orbit_grpc_protos::ClientCaptureEvent&& event) override;

  // Functions needed by the CaptureUploader to upload data.
  orbit_capture_uploader::DataReadiness GetDataReadiness() const override;
  void RefreshUploadDataBuffer() override;
  const std::vector<unsigned char>& GetUploadDataBuffer() const override {
    return capture_data_to_upload_;
  }

  // Functions needed by the tests.
  [[nodiscard]] size_t GetTotalUploadedEventCount() const { return total_uploaded_event_count_; }
  [[nodiscard]] size_t GetTotalUploadedDataBytes() const { return total_uploaded_data_bytes_; }

 private:
  // Close the output stream to stop the collector from adding new events.
  void Stop();

  mutable absl::Mutex mutex_;
  size_t buffered_event_count_ ABSL_GUARDED_BY(mutex_) = 0;
  size_t buffered_event_bytes_ ABSL_GUARDED_BY(mutex_) = 0;
  // Protect output_stream_ with mutex_ so that we can use output_stream_->IsOpen() in Conditions
  // for Await/LockWhen.
  std::unique_ptr<orbit_capture_file::CaptureFileOutputStream> output_stream_
      ABSL_GUARDED_BY(mutex_);
  orbit_capture_file::BufferOutputStream capture_data_buffer_stream_;
  std::vector<unsigned char> capture_data_to_upload_;

  size_t total_write_error_count_ = 0;
  size_t total_uploaded_event_count_ = 0;
  size_t total_uploaded_data_bytes_ = 0;
};

}  // namespace orbit_producer_event_processor

#endif  // CAPTURE_EVENT_PROCESSOR_UPLOADER_CAPTURE_EVENT_COLLECTOR_H_
