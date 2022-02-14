// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_
#define CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "CaptureServiceBase/ClientCaptureEventCollectorBuilder.h"
#include "CaptureUploader/UploadDataInterface.h"

namespace orbit_capture_service_base {

// A `ClientCaptureEventCollectorBuilder` implementation to build
// `UploaderClientCaptureEventCollector` and provide access to a `UploadDataInterface` for the cloud
// collector.
class UploaderClientCaptureEventCollectorBuilder : public ClientCaptureEventCollectorBuilder {
 public:
  [[nodiscard]] std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
  BuildClientCaptureEventCollector() override;

  // This method is blocked until BuildClientCaptureEventCollector is called.
  [[nodiscard]] orbit_capture_uploader::UploadDataInterface* GetUploadDataInterface();

 private:
  mutable absl::Mutex mutex_;
  orbit_capture_uploader::UploadDataInterface* upload_data_interface_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_