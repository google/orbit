// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_METRICS_UPLOADER_METRICS_UPLOADER_H_
#define ORBIT_METRICS_UPLOADER_METRICS_UPLOADER_H_

#ifdef _WIN32
#include <windows.h>
#endif  // WIN32

#include <chrono>
#include <cstdint>
#include <string>

#include "MetricsUploader/Result.h"
#include "OrbitBase/Result.h"
#include "orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

const std::string kMetricsUploaderClientDllName = "metrics_uploader_client";

// This class is used for sending log events from Orbit. It works only on Windows and only if
// metrics_uploader_client.dll is available. The types of logs that could be send are defined in
// orbit_client_protos::OrbitLogEvent::LogEventType.
//
// Only one instance of the class is allowed.
//
// Usage example:
//
// auto metrics_uploader = MetricsUploader::CreateMetricsUploader();
// if (metrics_uploader.has_value()) {
//   metrics_uploader.value().SendLogEvent(...);
// }
//
class MetricsUploader {
 public:
  MetricsUploader() = default;
  virtual ~MetricsUploader() = default;

  // Create a MetricsUploader instance, load metrics uploader client library if available and starts
  // metrics uploader client when called first time. Return the instance if there are no errors and
  // ErrorMessage otherwise.
  [[nodiscard]] static ErrorMessageOr<std::unique_ptr<MetricsUploader>> CreateMetricsUploader(
      std::string client_name = kMetricsUploaderClientDllName);

  // Send a log event to the server using metrics_uploader. Returns true on success and false
  // otherwise.
  virtual bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type) = 0;
  // Send a log event with an associated duration using metrics_uploader. Returns true on success
  // and false otherwise.
  virtual bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                            std::chrono::milliseconds event_duration) = 0;
  // Send a log event with an associated duration and status code using metrics_uploader. Returns
  // true on success and false otherwise.
  virtual bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                            std::chrono::milliseconds event_duration,
                            OrbitLogEvent_StatusCode status_code) = 0;
};

[[nodiscard]] ErrorMessageOr<std::string> GenerateUUID();

}  // namespace orbit_metrics_uploader

#endif  // ORBIT_METRICS_UPLOADER_METRICS_UPLOADER_H_
