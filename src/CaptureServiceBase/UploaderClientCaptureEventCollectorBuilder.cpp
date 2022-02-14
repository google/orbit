// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureServiceBase/UploaderClientCaptureEventCollectorBuilder.h"

#include "ProducerEventProcessor/UploaderClientCaptureEventCollector.h"

using orbit_capture_uploader::UploadDataInterface;
using orbit_producer_event_processor::ClientCaptureEventCollector;
using orbit_producer_event_processor::UploaderClientCaptureEventCollector;

namespace orbit_capture_service_base {

std::unique_ptr<ClientCaptureEventCollector>
UploaderClientCaptureEventCollectorBuilder::BuildClientCaptureEventCollector() {
  std::unique_ptr<UploaderClientCaptureEventCollector> uploader_client_capture_event_collector =
      std::make_unique<UploaderClientCaptureEventCollector>();

  absl::MutexLock lock{&mutex_};
  upload_data_interface_ = uploader_client_capture_event_collector.get();
  return std::move(uploader_client_capture_event_collector);
}

UploadDataInterface* UploaderClientCaptureEventCollectorBuilder::GetUploadDataInterface() {
  absl::MutexLock lock{&mutex_};
  mutex_.Await(absl::Condition(
      +[](UploadDataInterface* upload_data_interface) { return upload_data_interface != nullptr; },
      upload_data_interface_));
  return upload_data_interface_;
}

}  // namespace orbit_capture_service_base