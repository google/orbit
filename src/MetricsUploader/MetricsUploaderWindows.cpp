// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Rpc.h>
#include <absl/memory/memory.h>

#include <type_traits>

#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitVersion/OrbitVersion.h"
#include "orbit_log_event.pb.h"

namespace orbit_metrics_uploader {

constexpr const char* kSendLogEventFunctionName = "SendOrbitLogEvent";
constexpr const char* kSetupConnectionFunctionName = "SetupConnection";
constexpr const char* kShutdownConnectionFunctionName = "ShutdownConnection";
constexpr const char* kClientLogFileSuffix = "Orbit";

class MetricsUploaderImpl : public MetricsUploader {
 public:
  explicit MetricsUploaderImpl(std::string client_name, std::string session_uuid,
                               Result (*send_log_event_addr)(const uint8_t*, int),
                               Result (*shutdown_connection)(),
                               HMODULE metrics_uploader_client_dll);
  MetricsUploaderImpl(const MetricsUploaderImpl& other) = delete;
  MetricsUploaderImpl(MetricsUploaderImpl&& other) noexcept;
  MetricsUploaderImpl& operator=(const MetricsUploaderImpl& other) = delete;
  MetricsUploaderImpl& operator=(MetricsUploaderImpl&& other) noexcept;

  // Unload metrics_uploader_client.dll
  ~MetricsUploaderImpl() override;

  bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type) override;
  bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                    std::chrono::milliseconds event_duration) override;
  bool SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                    std::chrono::milliseconds event_duration,
                    OrbitLogEvent_StatusCode status_code) override;

 private:
  [[nodiscard]] bool FillAndSendLogEvent(OrbitLogEvent partial_filled_event) const;

  HMODULE metrics_uploader_client_dll_;
  Result (*send_log_event_addr_)(const uint8_t*, int) = nullptr;
  Result (*shutdown_connection_addr_)() = nullptr;
  std::string client_name_;
  std::string session_uuid_;
};

MetricsUploaderImpl::MetricsUploaderImpl(std::string client_name, std::string session_uuid,
                                         Result (*send_log_event_addr)(const uint8_t*, int),
                                         Result (*shutdown_connection_addr)(),
                                         HMODULE metrics_uploader_client_dll)
    : client_name_(std::move(client_name)),
      session_uuid_(std::move(session_uuid)),
      send_log_event_addr_(send_log_event_addr),
      shutdown_connection_addr_(shutdown_connection_addr),
      metrics_uploader_client_dll_(metrics_uploader_client_dll) {}

MetricsUploaderImpl::MetricsUploaderImpl(MetricsUploaderImpl&& other) noexcept
    : client_name_(std::move(other.client_name_)),
      session_uuid_(std::move(other.session_uuid_)),
      metrics_uploader_client_dll_(other.metrics_uploader_client_dll_),
      send_log_event_addr_(other.send_log_event_addr_),
      shutdown_connection_addr_(other.shutdown_connection_addr_) {
  other.metrics_uploader_client_dll_ = nullptr;
  other.send_log_event_addr_ = nullptr;
  other.shutdown_connection_addr_ = nullptr;
}

MetricsUploaderImpl& MetricsUploaderImpl::operator=(MetricsUploaderImpl&& other) noexcept {
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

MetricsUploaderImpl::~MetricsUploaderImpl() {
  if (nullptr != shutdown_connection_addr_) {
    Result result = shutdown_connection_addr_();
    if (result != kNoError) {
      ERROR("Error while closing connection: %s", GetErrorMessage(result));
    }
  }
  // Here FreeLibrary should be called. However, calling it on go-shared library leads to crash.
  // This should be revisited as soon as the issue in golang is fixed.
  //
  // if (nullptr != metrics_uploader_client_dll_) {
  //   FreeLibrary(metrics_uploader_client_dll_);
  // }
}

ErrorMessageOr<std::unique_ptr<MetricsUploader>> MetricsUploader::CreateMetricsUploader(
    std::string client_name) {
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

  auto setup_connection_addr = reinterpret_cast<Result (*)(const char*)>(
      GetProcAddress(metrics_uploader_client_dll, kSetupConnectionFunctionName));
  if (nullptr == setup_connection_addr) {
    // Here and below FreeLibrary should be called. However, calling it on go-shared library leads
    // to crash. This should be revisited as soon as the issue in golang is fixed.
    //
    // FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("%s function not found", kSetupConnectionFunctionName));
  }

  auto shutdown_connection_addr = reinterpret_cast<Result (*)()>(
      GetProcAddress(metrics_uploader_client_dll, kShutdownConnectionFunctionName));
  if (nullptr == shutdown_connection_addr) {
    // FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("%s function not found", kShutdownConnectionFunctionName));
  }

  auto send_log_event_addr = reinterpret_cast<Result (*)(const uint8_t*, int)>(
      GetProcAddress(metrics_uploader_client_dll, kSendLogEventFunctionName));
  if (nullptr == send_log_event_addr) {
    // FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("%s function not found", kSendLogEventFunctionName));
  }

  // set up connection and create a client
  Result result = setup_connection_addr(kClientLogFileSuffix);
  if (result != kNoError) {
    if (result != kCannotOpenConnection) {
      Result shutdown_result = shutdown_connection_addr();
      if (shutdown_result != kNoError) {
        ERROR("Error while closing connection: %s", GetErrorMessage(shutdown_result));
      }
    }
    // FreeLibrary(metrics_uploader_client_dll);
    return ErrorMessage(absl::StrFormat("Error while starting the metrics uploader client: %s",
                                        GetErrorMessage(result)));
  }

  return std::make_unique<MetricsUploaderImpl>(client_name, session_uuid, send_log_event_addr,
                                               shutdown_connection_addr,
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

bool MetricsUploaderImpl::FillAndSendLogEvent(OrbitLogEvent partial_filled_event) const {
  if (send_log_event_addr_ == nullptr) {
    ERROR("Unable to send metric, send_log_event_addr_ is nullptr");
    return false;
  }

  partial_filled_event.set_orbit_version(orbit_core::GetVersion());
  partial_filled_event.set_session_uuid(session_uuid_);

  int message_size = partial_filled_event.ByteSize();
  std::vector<uint8_t> buffer(message_size);
  partial_filled_event.SerializeToArray(buffer.data(), message_size);

  Result result = send_log_event_addr_(buffer.data(), message_size);
  if (result != kNoError) {
    ERROR("Unable to send metrics event: %s", GetErrorMessage(result));
    return false;
  }

  return true;
}

bool MetricsUploaderImpl::SendLogEvent(OrbitLogEvent_LogEventType log_event_type) {
  OrbitLogEvent log_event;
  log_event.set_log_event_type(log_event_type);
  return FillAndSendLogEvent(std::move(log_event));
}

bool MetricsUploaderImpl::SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                                       std::chrono::milliseconds event_duration) {
  OrbitLogEvent log_event;
  log_event.set_log_event_type(log_event_type);
  log_event.set_event_duration_milliseconds(event_duration.count());
  return FillAndSendLogEvent(std::move(log_event));
}

bool MetricsUploaderImpl::SendLogEvent(OrbitLogEvent_LogEventType log_event_type,
                                       std::chrono::milliseconds event_duration,
                                       OrbitLogEvent_StatusCode status_code) {
  OrbitLogEvent log_event;
  log_event.set_log_event_type(log_event_type);
  log_event.set_event_duration_milliseconds(event_duration.count());
  log_event.set_status_code(status_code);
  return FillAndSendLogEvent(log_event);
}

}  // namespace orbit_metrics_uploader
