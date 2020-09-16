// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGrpcServer.h"

#include <absl/flags/flag.h>

#include "CaptureServiceImpl.h"
#include "CrashServiceImpl.h"
#include "FramePointerValidatorServiceImpl.h"
#include "ProcessServiceImpl.h"
#include "TracepointServiceImpl.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"

ABSL_DECLARE_FLAG(bool, devmode);

namespace orbit_service {

namespace {

class OrbitGrpcServerImpl final : public OrbitGrpcServer {
 public:
  OrbitGrpcServerImpl() = default;
  OrbitGrpcServerImpl(const OrbitGrpcServerImpl&) = delete;
  OrbitGrpcServerImpl& operator=(OrbitGrpcServerImpl&) = delete;

  [[nodiscard]] bool Init(std::string_view server_address);

  void Shutdown() override;
  void Wait() override;

 private:
  CaptureServiceImpl capture_service_;
  ProcessServiceImpl process_service_;
  TracepointServiceImpl tracepoint_service_;
  FramePointerValidatorServiceImpl frame_pointer_validator_service_;
  CrashServiceImpl crash_service_;
  std::unique_ptr<grpc::Server> server_;
};

bool OrbitGrpcServerImpl::Init(std::string_view server_address) {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  grpc::ServerBuilder builder;

  builder.AddListeningPort(std::string(server_address), grpc::InsecureServerCredentials());
  builder.RegisterService(&capture_service_);
  builder.RegisterService(&process_service_);
  builder.RegisterService(&tracepoint_service_);
  builder.RegisterService(&frame_pointer_validator_service_);
  if (absl::GetFlag(FLAGS_devmode)) {
    builder.RegisterService(&crash_service_);
  }

  server_ = builder.BuildAndStart();

  return server_ != nullptr;
}

void OrbitGrpcServerImpl::Shutdown() { server_->Shutdown(); }

void OrbitGrpcServerImpl::Wait() { server_->Wait(); }

}  // namespace

std::unique_ptr<OrbitGrpcServer> OrbitGrpcServer::Create(std::string_view server_address) {
  std::unique_ptr<OrbitGrpcServerImpl> server_impl = std::make_unique<OrbitGrpcServerImpl>();

  if (!server_impl->Init(server_address)) {
    return nullptr;
  }

  return std::move(server_impl);
}

}  // namespace orbit_service