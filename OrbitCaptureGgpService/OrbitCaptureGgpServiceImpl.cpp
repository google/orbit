// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpServiceImpl.h"

#include <memory>
#include <string>

#include "OrbitClientGgp/ClientGgp.h"
#include "absl/flags/flag.h"

ABSL_DECLARE_FLAG(uint16_t, orbit_service_grpc_port);
ABSL_DECLARE_FLAG(int32_t, pid);
ABSL_DECLARE_FLAG(std::vector<std::string>, functions);
ABSL_DECLARE_FLAG(std::string, file_name);
ABSL_DECLARE_FLAG(std::string, file_directory);

using grpc::ServerContext;
using grpc::Status;
using grpc::StatusCode;

CaptureClientGgpServiceImpl::CaptureClientGgpServiceImpl()
    : orbit_grpc_protos::CaptureClientGgpService::Service{} {
  InitClientGgp();
}

Status CaptureClientGgpServiceImpl::SayHello(grpc::ServerContext*,
                                             const orbit_grpc_protos::HelloRequest* request,
                                             orbit_grpc_protos::HelloReply* reply) {
  std::string prefix("Hello ");
  reply->set_message(prefix + request->name());
  return Status::OK;
}

void CaptureClientGgpServiceImpl::InitClientGgp() {
  LOG("Initialise ClientGgp");
  ClientGgpOptions client_ggp_options;
  uint64_t orbit_service_grpc_port = absl::GetFlag(FLAGS_orbit_service_grpc_port);
  client_ggp_options.grpc_server_address = absl::StrFormat("127.0.0.1:%d", orbit_service_grpc_port);
  client_ggp_options.capture_pid = absl::GetFlag(FLAGS_pid);
  client_ggp_options.capture_functions = absl::GetFlag(FLAGS_functions);
  client_ggp_options.capture_file_name = absl::GetFlag(FLAGS_file_name);
  client_ggp_options.capture_file_directory = absl::GetFlag(FLAGS_file_directory);

  client_ggp_ = ClientGgp(std::move(client_ggp_options));
  if (!client_ggp_.InitClient()) {
    ERROR("Not possible to initialise client");
    return;
  }
  LOG("ClientGgp Initialised");
  return;
}