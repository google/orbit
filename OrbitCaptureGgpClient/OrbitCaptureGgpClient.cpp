// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "services_ggp.grpc.pb.h"

using orbit_grpc_protos::ShutdownServiceRequest;
using orbit_grpc_protos::ShutdownServiceResponse;
using orbit_grpc_protos::StartCaptureRequest;
using orbit_grpc_protos::StartCaptureResponse;
using orbit_grpc_protos::StopAndSaveCaptureRequest;
using orbit_grpc_protos::StopAndSaveCaptureResponse;
using orbit_grpc_protos::UpdateSelectedFunctionsRequest;
using orbit_grpc_protos::UpdateSelectedFunctionsResponse;

using grpc::ClientContext;
using grpc::Status;

ErrorMessageOr<void> CaptureClientGgpClient::StartCapture() {
  StartCaptureRequest request;
  StartCaptureResponse response;
  auto context = std::make_unique<ClientContext>();

  Status status = capture_client_ggp_service_->StartCapture(context.get(), request, &response);

  if (!status.ok()) {
    ERROR("gRPC call to StartCapture failed: %s (error_code=%d)", status.error_message(),
          status.error_code());
    return ErrorMessage(status.error_message());
  }
  LOG("Capture started");
  return outcome::success();
}

ErrorMessageOr<void> CaptureClientGgpClient::StopAndSaveCapture() {
  StopAndSaveCaptureRequest request;
  StopAndSaveCaptureResponse response;
  auto context = std::make_unique<ClientContext>();

  Status status =
      capture_client_ggp_service_->StopAndSaveCapture(context.get(), request, &response);
  if (!status.ok()) {
    ERROR("gRPC call to StopCapture failed: %s (error_code=%d)", status.error_message(),
          status.error_code());
    return ErrorMessage(status.error_message());
  }
  LOG("Capture finished");
  return outcome::success();
}

ErrorMessageOr<void> CaptureClientGgpClient::UpdateSelectedFunctions(
    std::vector<std::string> selected_functions) {
  UpdateSelectedFunctionsRequest request;
  UpdateSelectedFunctionsResponse response;
  auto context = std::make_unique<ClientContext>();
  for (const std::string& function : selected_functions) {
    request.add_functions(function);
  }

  Status status =
      capture_client_ggp_service_->UpdateSelectedFunctions(context.get(), request, &response);
  if (!status.ok()) {
    ERROR("gRPC call to UpdateSelectedFunctions failed: %s (error_code=%d)", status.error_message(),
          status.error_code());
    return ErrorMessage(status.error_message());
  }
  LOG("Functions updated");
  return outcome::success();
}

void CaptureClientGgpClient::ShutdownService() {
  ShutdownServiceRequest request;
  ShutdownServiceResponse response;
  auto context = std::make_unique<ClientContext>();

  capture_client_ggp_service_->ShutdownService(context.get(), request, &response);
}