// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/UploaderClientCaptureEventCollectorManager.h"

using orbit_capture_uploader::UploadDataInterface;
using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::UploaderClientCaptureEventCollector;

namespace orbit_capture_service_base {

UploaderClientCaptureEventCollectorManager::UploaderClientCaptureEventCollectorManager()
    : uploader_client_capture_event_collector_{
          std::make_unique<UploaderClientCaptureEventCollector>()} {}

ClientCaptureEventCollector*
UploaderClientCaptureEventCollectorManager::GetClientCaptureEventCollector() {
  absl::MutexLock lock{&mutex_};
  return uploader_client_capture_event_collector_.get();
}

UploadDataInterface* UploaderClientCaptureEventCollectorManager::GetUploadDataInterface() {
  absl::MutexLock lock{&mutex_};
  return uploader_client_capture_event_collector_.get();
}

}  // namespace orbit_capture_service_base