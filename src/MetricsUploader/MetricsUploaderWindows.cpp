// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/memory/memory.h>

#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Logging.h"

namespace orbit_metrics_uploader {

constexpr const char* kSendLogEventFunctionName = "SendOrbitLogEvent";
constexpr const char* kStartUploaderClientFunctionName = "StartUploaderClient";

MetricsUploader::MetricsUploader(std::string client_name,
                                 Result (*send_log_event_addr)(const uint8_t*, int),
                                 HMODULE metrics_uploader_client_dll)
    : client_name_(std::move(client_name)),
      send_log_event_addr_(send_log_event_addr),
      metrics_uploader_client_dll_(metrics_uploader_client_dll) {}

MetricsUploader::MetricsUploader(MetricsUploader&& other)
    : client_name_(std::move(other.client_name_)),
      metrics_uploader_client_dll_(other.metrics_uploader_client_dll_),
      send_log_event_addr_(other.send_log_event_addr_) {
  other.metrics_uploader_client_dll_ = nullptr;
  other.send_log_event_addr_ = nullptr;
}

MetricsUploader& MetricsUploader::operator=(MetricsUploader&& other) {
  if (&other == this) {
    return *this;
  }

  client_name_ = std::move(other.client_name_);

  metrics_uploader_client_dll_ = other.metrics_uploader_client_dll_;
  other.metrics_uploader_client_dll_ = nullptr;

  send_log_event_addr_ = other.send_log_event_addr_;
  other.send_log_event_addr_ = nullptr;

  return *this;
}

MetricsUploader::~MetricsUploader() {
  if (nullptr != metrics_uploader_client_dll_) {
    FreeLibrary(metrics_uploader_client_dll_);
  }
}

ErrorMessageOr<MetricsUploader> MetricsUploader::CreateMetricsUploader(std::string client_name) {
  HMODULE metrics_uploader_client_dll;
  if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, client_name.c_str(),
                         &metrics_uploader_client_dll) != 0) {
    return ErrorMessage("MetricsUploader is already created");
  }

  metrics_uploader_client_dll = LoadLibraryA(client_name.c_str());
  if (nullptr != metrics_uploader_client_dll) {
    auto start_uploader_client_addr = reinterpret_cast<Result (*)()>(
        GetProcAddress(metrics_uploader_client_dll, kStartUploaderClientFunctionName));
    if (nullptr == start_uploader_client_addr) {
      FreeLibrary(metrics_uploader_client_dll);
      return ErrorMessage(
          absl::StrFormat("%s function not found", kStartUploaderClientFunctionName));
    }

    auto send_log_event_addr = reinterpret_cast<Result (*)(const uint8_t*, int)>(
        GetProcAddress(metrics_uploader_client_dll, kSendLogEventFunctionName));
    if (nullptr == send_log_event_addr) {
      FreeLibrary(metrics_uploader_client_dll);
      return ErrorMessage(absl::StrFormat("%s function not found", kSendLogEventFunctionName));
    }

    // set up connection and create a client
    Result result = start_uploader_client_addr();
    if (result != kNoError) {
      FreeLibrary(metrics_uploader_client_dll);
      return ErrorMessage(absl::StrFormat("Error while starting the metrics uploader client: %s",
                                          GetErrorMessage(result)));
    }

    return MetricsUploader(client_name, send_log_event_addr, metrics_uploader_client_dll);
  }
  return ErrorMessage("Metrics uploader client library is not found");
}

}  // namespace orbit_metrics_uploader
