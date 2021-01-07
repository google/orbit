// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MetricsUploader/MetricsUploader.h"

#include <vector>

#include "OrbitBase/Logging.h"

namespace orbit_metrics_uploader {

bool MetricsUploader::SendLogEvent(OrbitLogEvent_LogEventType log_event_type) {
  if (send_log_event_addr_ != nullptr) {
    OrbitLogEvent log_event;
    log_event.set_log_event_type(log_event_type);

    int message_size = log_event.ByteSize();
    std::vector<uint8_t> buffer(message_size);
    log_event.SerializeToArray(buffer.data(), message_size);

    Result result = send_log_event_addr_(buffer.data(), message_size);
    if (result == kNoError) {
      return true;
    }
    ERROR("Can't start the metrics uploader client: %s", GetErrorMessage(result));
  }
  return false;
}

}  // namespace orbit_metrics_uploader
