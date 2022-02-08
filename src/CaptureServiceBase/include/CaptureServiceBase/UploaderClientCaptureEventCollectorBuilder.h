// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_
#define CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_

#include "CaptureServiceBase/ClientCaptureEventCollectorBuilder.h"

namespace orbit_capture_service_base {

// Create a `ClientCaptureEventCollectorBuilder` which builds a `UploaderCaptureEventCollector` for
// the cloud collector.
std::unique_ptr<ClientCaptureEventCollectorBuilder>
CreateUploaderClientCaptureEventCollectorBuilder();

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_UPLOADER_CLIENT_CAPTURE_EVENT_COLLECTOR_BUILDER_H_