// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Rpc.h>
#include <absl/memory/memory.h>

#include <type_traits>

#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_metrics_uploader {

constexpr const char* kSendLogEventFunctionName = "SendOrbitLogEvent";
constexpr const char* kSetupConnectionFunctionName = "SetupConnection";
constexpr const char* kShutdownConnectionFunctionName = "ShutdownConnection";

MetricsUploader::MetricsUploader(std::string client_name, std::string session_uuid,
                                 Result (*send_log_event_addr)(const uint8_t*, int),
                                 Result (*shutdown_connection_addr)(),
                                 HMODULE metrics_uploader_client_dll)
    : client_name_(std::move(client_name)),
      session_uuid_(std::move(session_uuid)),
      send_log_event_addr_(send_log_event_addr),
      shutdown_connection_addr_(shutdown_connection_addr),
      metrics_uploader_client_dll_(metrics_uploader_client_dll) {}

MetricsUploader::MetricsUploader(MetricsUploader&& other)
    : client_name_(std::move(other.client_name_)),
      session_uuid_(std::move(other.session_uuid_)),
      metrics_uploader_client_dll_(other.metrics_uploader_client_dll_),
      send_log_event_addr_(other.send_log_event_addr_),
      shutdown_connection_addr_(other.shutdown_connection_addr_) {
  other.metrics_uploader_client_dll_ = nullptr;
  other.send_log_event_addr_ = nullptr;
  other.shutdown_connection_addr_ = nullptr;
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

  shutdown_connection_addr_ = other.shutdown_connection_addr_;
  other.shutdown_connection_addr_ = nullptr;

  return *this;
}

MetricsUploader::~MetricsUploader() {
  if (nullptr != shutdown_connection_addr_) {
    Result result = shutdown_connection_addr_();
    if (result != kNoError) {
      ERROR("Error while closing connection: %s", GetErrorMessage(result));
    }
  }
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

  OUTCOME_TRY(session_uuid, GenerateUUID());
  LOG("Session UUID for metrics: %s", session_uuid);

  metrics_uploader_client_dll = LoadLibraryA(client_name.c_str());
  if (metrics_uploader_client_dll == nullptr) {
    return ErrorMessage("Metrics uploader client library is not found");
  }

  auto setup_connection_addr = reinterpret_cast<Result (*)()>(
      GetProcAddress(metrics_uploader_client_dll, kSetupConnectionFunctionName));
  if (nullptr == setup_connection_addr) {
    FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("%s function not found", kSetupConnectionFunctionName));
  }

  auto shutdown_connection_addr = reinterpret_cast<Result (*)()>(
      GetProcAddress(metrics_uploader_client_dll, kShutdownConnectionFunctionName));
  if (nullptr == shutdown_connection_addr) {
    FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("%s function not found", kShutdownConnectionFunctionName));
  }

  auto send_log_event_addr = reinterpret_cast<Result (*)(const uint8_t*, int)>(
      GetProcAddress(metrics_uploader_client_dll, kSendLogEventFunctionName));
  if (nullptr == send_log_event_addr) {
    FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("%s function not found", kSendLogEventFunctionName));
  }

  // set up connection and create a client
  Result result = setup_connection_addr();
  if (result != kNoError) {
    Result result = shutdown_connection_addr();
    if (result != kNoError) {
      ERROR("Error while closing connection: %s", GetErrorMessage(result));
    }
    FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("Error while starting the metrics uploader client: %s",
                                        GetErrorMessage(result)));
  }

  return MetricsUploader(client_name, session_uuid, send_log_event_addr, shutdown_connection_addr,
                         metrics_uploader_client_dll);
}

ErrorMessageOr<std::string> GenerateUUID() {
  UUID uuid;
  RPC_STATUS create_status = UuidCreate(&uuid);
  if (create_status != RPC_S_OK) {
    return ErrorMessage("Unable to create UUID for metrics uploader");
  }
  RPC_CSTR uuid_c_str = nullptr;
  RPC_STATUS convert_status = UuidToStringA(&uuid, &uuid_c_str);
  if (convert_status != RPC_S_OK) {
    return ErrorMessage("Unable to convert UUID to string for metrics uploader");
  }

  static_assert(std::is_same_v<RPC_CSTR, unsigned char*>,
                "The type of RPC_CSTR needs to be castable to const char*");

  std::string uuid_string{reinterpret_cast<const char*>(uuid_c_str)};
  RpcStringFreeA(&uuid_c_str);

  return uuid_string;
}

}  // namespace orbit_metrics_uploader
