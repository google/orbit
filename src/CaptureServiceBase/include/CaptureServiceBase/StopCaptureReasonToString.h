// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_STOP_CAPTURE_REASON_TO_STRING_H_
#define CAPTURE_SERVICE_BASE_STOP_CAPTURE_REASON_TO_STRING_H_

#include <string>

#include "CaptureServiceBase/CaptureServiceBase.h"
#include "OrbitBase/Logging.h"

namespace orbit_capture_service_base {

[[nodiscard]] std::string StopCaptureReasonToString(
    OrbitCaptureBase::StopCaptureReason stop_capture_reason) {
  switch (stop_capture_reason) {
    case StopCaptureReason::kClientStop:
      return "client_stop";
    case StopCaptureReason::kMemoryWatchdog:
      return "memory_watchdog";
    case StopCaptureReason::kExceededMaxDurationLimit:
      return "exceeded_max_duration_limit";
    case StopCaptureReason::kGuestOrcStop:
      return "guestorc_stop";
    case StopCaptureReason::kGuestOrcConnectionFailure:
      return "guestorc_connection_failure";
  }

  ORBIT_UNREACHABLE();
}

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_STOP_CAPTURE_REASON_TO_STRINGE_H_