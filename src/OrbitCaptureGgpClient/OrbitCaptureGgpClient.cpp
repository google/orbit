// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"

#include <absl/types/span.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/channel_arguments.h>
#include <stdint.h>

#include <limits>
#include <memory>
#include <string>
#include <string_view>

#include "GrpcProtos/services_ggp.grpc.pb.h"
#include "GrpcProtos/services_ggp.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

using orbit_grpc_protos::ShutdownServiceRequest;
using orbit_grpc_protos::ShutdownServiceResponse;
using orbit_grpc_protos::StartCaptureRequest;
using orbit_grpc_protos::StartCaptureResponse;
using orbit_grpc_protos::StopCaptureRequest;
using orbit_grpc_protos::StopCaptureResponse;
using orbit_grpc_protos::UpdateSelectedFunctionsRequest;
using orbit_grpc_protos::UpdateSelectedFunctionsResponse;

using grpc::ClientContext;
using grpc::Status;

class CaptureClientGgpClient::CaptureClientGgpClientImpl {
 public:
  void SetupGrpcClient(std::string_view grpc_server_address);

  [[nodiscard]] ErrorMessageOr<void> StartCapture();
  [[nodiscard]] ErrorMessageOr<void> StopCapture();
  [[nodiscard]] ErrorMessageOr<void> UpdateSelectedFunctions(
      absl::Span<const std::string> selected_functions);
  void ShutdownService();

 private:
  std::unique_ptr<orbit_grpc_protos::CaptureClientGgpService::Stub> capture_client_ggp_service_;
};

CaptureClientGgpClient::CaptureClientGgpClient(std::string_view grpc_server_address)
    : pimpl{std::make_unique<CaptureClientGgpClientImpl>()} {
  pimpl->SetupGrpcClient(grpc_server_address);
}

int CaptureClientGgpClient::StartCapture() {
  ErrorMessageOr<void> result = pimpl->StartCapture();
  if (result.has_error()) {
    ORBIT_ERROR("Not possible to start capture: %s", result.error().message());
    return 0;
  }
  return 1;
}

int CaptureClientGgpClient::StopCapture() {
  ErrorMessageOr<void> result = pimpl->StopCapture();
  if (result.has_error()) {
    ORBIT_ERROR("Not possible to stop or save capture: %s", result.error().message());
    return 0;
  }
  return 1;
}

int CaptureClientGgpClient::UpdateSelectedFunctions(
    absl::Span<const std::string> selected_functions) {
  ErrorMessageOr<void> result = pimpl->UpdateSelectedFunctions(selected_functions);
  if (result.has_error()) {
    ORBIT_ERROR("Not possible to update functions %s", result.error().message());
    return 0;
  }
  return 1;
}

void CaptureClientGgpClient::ShutdownService() { pimpl->ShutdownService(); }

CaptureClientGgpClient::~CaptureClientGgpClient() = default;
[[maybe_unused]] CaptureClientGgpClient::CaptureClientGgpClient(CaptureClientGgpClient&&) = default;
CaptureClientGgpClient& CaptureClientGgpClient::operator=(CaptureClientGgpClient&&) = default;

void CaptureClientGgpClient::CaptureClientGgpClientImpl::SetupGrpcClient(
    std::string_view grpc_server_address) {
  grpc::ChannelArguments channel_arguments;
  channel_arguments.SetMaxReceiveMessageSize(std::numeric_limits<int32_t>::max());

  std::shared_ptr<::grpc::Channel> grpc_channel = grpc::CreateCustomChannel(
      std::string{grpc_server_address}, grpc::InsecureChannelCredentials(), channel_arguments);
  if (!grpc_channel) {
    ORBIT_ERROR("Unable to create GRPC channel to %s", grpc_server_address);
    return;
  }
  ORBIT_LOG("Created GRPC channel to %s", grpc_server_address);

  capture_client_ggp_service_ = orbit_grpc_protos::CaptureClientGgpService::NewStub(grpc_channel);
}

ErrorMessageOr<void> CaptureClientGgpClient::CaptureClientGgpClientImpl::StartCapture() {
  StartCaptureRequest request;
  StartCaptureResponse response;
  auto context = std::make_unique<ClientContext>();

  Status status = capture_client_ggp_service_->StartCapture(context.get(), request, &response);

  if (!status.ok()) {
    ORBIT_ERROR("gRPC call to StartCapture failed: %s (error_code=%d)", status.error_message(),
                status.error_code());
    return ErrorMessage(status.error_message());
  }
  ORBIT_LOG("Capture started");
  return outcome::success();
}

ErrorMessageOr<void> CaptureClientGgpClient::CaptureClientGgpClientImpl::StopCapture() {
  StopCaptureRequest request;
  StopCaptureResponse response;
  auto context = std::make_unique<ClientContext>();

  Status status = capture_client_ggp_service_->StopCapture(context.get(), request, &response);
  if (!status.ok()) {
    ORBIT_ERROR("gRPC call to StopCapture failed: %s (error_code=%d)", status.error_message(),
                status.error_code());
    return ErrorMessage(status.error_message());
  }
  ORBIT_LOG("Capture finished");
  return outcome::success();
}

ErrorMessageOr<void> CaptureClientGgpClient::CaptureClientGgpClientImpl::UpdateSelectedFunctions(
    absl::Span<const std::string> selected_functions) {
  UpdateSelectedFunctionsRequest request;
  UpdateSelectedFunctionsResponse response;
  auto context = std::make_unique<ClientContext>();
  for (const std::string& function : selected_functions) {
    request.add_functions(function);
  }

  Status status =
      capture_client_ggp_service_->UpdateSelectedFunctions(context.get(), request, &response);
  if (!status.ok()) {
    ORBIT_ERROR("gRPC call to UpdateSelectedFunctions failed: %s (error_code=%d)",
                status.error_message(), status.error_code());
    return ErrorMessage(status.error_message());
  }
  ORBIT_LOG("Functions updated");
  return outcome::success();
}

void CaptureClientGgpClient::CaptureClientGgpClientImpl::CaptureClientGgpClientImpl::
    ShutdownService() {
  ShutdownServiceRequest request;
  ShutdownServiceResponse response;
  auto context = std::make_unique<ClientContext>();

  capture_client_ggp_service_->ShutdownService(context.get(), request, &response);
}