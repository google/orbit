// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include <cstdint>

#include "MetricsUploader/Result.h"
#include "orbit_log_event.pb.h"

using orbit_metrics_uploader::OrbitLogEvent;
using orbit_metrics_uploader::OrbitLogEvent_LogEventType_UNKNOWN_EVENT_TYPE;

using orbit_metrics_uploader::Result;

extern "C" {
__declspec(dllexport) enum Result SetupConnection() { return Result::kNoError; }
__declspec(dllexport) enum Result ShutdownConnection() { return Result::kNoError; }
__declspec(dllexport) enum Result SendOrbitLogEvent(uint8_t* serialized_proto, int length) {
  OrbitLogEvent log_event;
  bool result = log_event.ParseFromArray(serialized_proto, length);
  if (!result) {
    return Result::kCannotUnmarshalLogEvent;
  }
  if (log_event.log_event_type() == OrbitLogEvent_LogEventType_UNKNOWN_EVENT_TYPE) {
    return Result::kCannotQueueLogEvent;
  }
  return Result::kNoError;
}
}  // extern "C"
