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
  // Create a MetricsUploader instance, load metrics uploader client library if available and starts
  // metrics uploader client when called first time. Return the instance if there are no errors and
  // ErrorMessage otherwise.
  [[nodiscard]] static ErrorMessageOr<MetricsUploader> CreateMetricsUploader(
      std::string client_name = kMetricsUploaderClientDllName);

  // Unload metrics_uploader_client.dll
  ~MetricsUploader();

  MetricsUploader(const MetricsUploader& other) = delete;
  MetricsUploader(MetricsUploader&& other);
  MetricsUploader& operator=(const MetricsUploader& other) = delete;
  MetricsUploader& operator=(MetricsUploader&& other);

  // Send log events to the server using metrics_uploader.
  // Returns true on success and false otherwise.
  bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                    std::chrono::milliseconds event_duration = std::chrono::milliseconds::zero());

 private:
#ifdef _WIN32
  HMODULE metrics_uploader_client_dll_;
  explicit MetricsUploader(std::string client_name, std::string session_uuid,
                           Result (*send_log_event_addr)(const uint8_t*, int),
                           HMODULE metrics_uploader_client_dll);
#else
  MetricsUploader();
#endif

  Result (*send_log_event_addr_)(const uint8_t*, int) = nullptr;
  std::string client_name_;
  std::string session_uuid_;
};

[[nodiscard]] ErrorMessageOr<std::string> GenerateUUID();

}  // namespace orbit_metrics_uploader

#endif  // ORBIT_METRICS_UPLOADER_METRICS_UPLOADER_H_
