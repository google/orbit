// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_
#define CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>

#include "CaptureServiceBase/ClientCaptureEventCollectorManager.h"
#include "CaptureUploader/UploadDataInterface.h"
#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"

namespace orbit_capture_service_base {

// A `ClientCaptureEventCollectorManager` implementation to build and manager a
// `UploaderClientCaptureEventCollector` for the cloud collector. It also provides access to a
// `UploadDataInterface`.
class UploaderClientCaptureEventCollectorManager : public ClientCaptureEventCollectorManager {
 public:
  explicit UploaderClientCaptureEventCollectorManager();

  [[nodiscard]] orbit_producer_event_processor::ClientCaptureEventCollector*
  GetClientCaptureEventCollector() override;

  [[nodiscard]] orbit_capture_uploader::UploadDataInterface* GetUploadDataInterface();

 private:
  mutable absl::Mutex mutex_;
  std::unique_ptr<orbit_producer_event_processor::UploaderClientCaptureEventCollector>
      uploader_client_capture_event_collector_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_MANAGER_H_