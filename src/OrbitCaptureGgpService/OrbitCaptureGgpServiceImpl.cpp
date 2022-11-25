// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpServiceImpl.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>
#include <absl/strings/str_format.h>
#include <absl/time/time.h>

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitClientGgp/ClientGgp.h"
#include "OrbitClientGgp/ClientGgpOptions.h"

ABSL_DECLARE_FLAG(uint16_t, orbit_service_grpc_port);
ABSL_DECLARE_FLAG(int32_t, pid);
ABSL_DECLARE_FLAG(std::vector<std::string>, functions);
ABSL_DECLARE_FLAG(std::string, file_name);
ABSL_DECLARE_FLAG(std::string, file_directory);

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

using orbit_grpc_protos::ShutdownServiceRequest;
using orbit_grpc_protos::ShutdownServiceResponse;
using orbit_grpc_protos::StartCaptureRequest;
using orbit_grpc_protos::StartCaptureResponse;
using orbit_grpc_protos::StopCaptureRequest;
using orbit_grpc_protos::StopCaptureResponse;
using orbit_grpc_protos::UpdateSelectedFunctionsRequest;
using orbit_grpc_protos::UpdateSelectedFunctionsResponse;

CaptureClientGgpServiceImpl::CaptureClientGgpServiceImpl()
    : orbit_grpc_protos::CaptureClientGgpService::Service{} {
  thread_pool_ = orbit_base::ThreadPool::Create(1, 1, absl::Seconds(1));
  InitClientGgp();
}

void CaptureClientGgpServiceImpl::InitClientGgp() {
  ORBIT_LOG("Initialise ClientGgp");
  ClientGgpOptions client_ggp_options;
  uint64_t orbit_service_grpc_port = absl::GetFlag(FLAGS_orbit_service_grpc_port);
  client_ggp_options.grpc_server_address = absl::StrFormat("127.0.0.1:%d", orbit_service_grpc_port);
  client_ggp_options.capture_pid = absl::GetFlag(FLAGS_pid);
  client_ggp_options.capture_functions = absl::GetFlag(FLAGS_functions);
  client_ggp_options.capture_file_name = absl::GetFlag(FLAGS_file_name);
  client_ggp_options.capture_file_directory = absl::GetFlag(FLAGS_file_directory);

  client_ggp_ = std::make_unique<ClientGgp>(std::move(client_ggp_options));
  if (!client_ggp_->InitClient()) {
    ORBIT_ERROR("Not possible to initialise client");
    return;
  }
  ORBIT_LOG("ClientGgp Initialised");
}

Status CaptureClientGgpServiceImpl::StartCapture(ServerContext* /*context*/,
                                                 const StartCaptureRequest* /*request*/,
                                                 StartCaptureResponse* /*response*/) {
  ORBIT_LOG("Start capture grpc call received");
  if (CaptureIsRunning()) {
    return Status(StatusCode::INTERNAL, "A capture is already running");
  }

  auto start_capture_result = client_ggp_->RequestStartCapture(thread_pool_.get());

  if (start_capture_result.has_error()) {
    return Status(StatusCode::INTERNAL, start_capture_result.error().message());
  }
  return Status::OK;
}

Status CaptureClientGgpServiceImpl::StopCapture(grpc::ServerContext* /*context*/,
                                                const StopCaptureRequest* /*request*/,
                                                StopCaptureResponse* /*response*/) {
  ORBIT_LOG("Stop capture grpc call received");
  if (!client_ggp_->StopCapture()) {
    return Status(StatusCode::INTERNAL, "Not possible to stop the capture");
  }
  // idle until all the capture data is received
  while (CaptureIsRunning()) {
  }
  return Status::OK;
}

Status CaptureClientGgpServiceImpl::UpdateSelectedFunctions(
    grpc::ServerContext* /*context*/, const UpdateSelectedFunctionsRequest* request,
    UpdateSelectedFunctionsResponse* /* response */) {
  ORBIT_LOG("UpdateSelectedFunctions grpc call received");
  std::vector<std::string> capture_functions;
  for (const auto& function : request->functions()) {
    capture_functions.push_back(function);
  }

  client_ggp_->UpdateCaptureFunctions(capture_functions);
  return Status::OK;
}

Status CaptureClientGgpServiceImpl::ShutdownService(grpc::ServerContext* /*context*/,
                                                    const ShutdownServiceRequest* /*request*/,
                                                    ShutdownServiceResponse* /*response*/) {
  ORBIT_LOG("Shutdown grpc call received");
  Shutdown();
  return Status::OK;
}

void CaptureClientGgpServiceImpl::Shutdown() {
  if (CaptureIsRunning()) {
    if (!client_ggp_->StopCapture()) {
      ORBIT_LOG("Not possible to stop the capture");
    }
  }
  ORBIT_LOG("Shut down the thread and wait for it to finish");
  thread_pool_->ShutdownAndWait();
  shutdown_finished_ = true;

  ORBIT_LOG("All done");
}

bool CaptureClientGgpServiceImpl::CaptureIsRunning() {
  return (thread_pool_->GetNumberOfBusyThreads() > 0);
}

bool CaptureClientGgpServiceImpl::ShutdownFinished() const { return shutdown_finished_; }