// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_
#define CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_

#include "CaptureServiceBase/StartStopCaptureRequestWaiter.h"

namespace orbit_capture_service_base {

// Create a `StartStopCaptureRequestWaiter` for the cloud collector.
[[nodiscard]] std::shared_ptr<StartStopCaptureRequestWaiter>
CreateCloudCollectorStartStopCaptureRequestWaiter();

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_CLOUD_COLLECTOR_START_STOP_CAPTURE_REQUEST_WAITER_H_