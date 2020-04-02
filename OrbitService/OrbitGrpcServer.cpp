// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGrpcServer.h"

#include "ProcessServiceImpl.h"
#include "grpcpp/grpcpp.h"
#include "grpcpp/health_check_service_interface.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

namespace {

class OrbitGrpcServerImpl final : public OrbitGrpcServer {
 public:
  OrbitGrpcServerImpl() = default;
  OrbitGrpcServerImpl(const OrbitGrpcServerImpl&) = delete;
  OrbitGrpcServerImpl& operator=(OrbitGrpcServerImpl&) = delete;

  void Init(std::string_view server_address);

  void Shutdown() override;
  void Wait() override;
 private:
  ProcessServiceImpl process_service_;
  std::unique_ptr<grpc::Server> server_;
};

void OrbitGrpcServerImpl::Init(std::string_view server_address) {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  grpc::ServerBuilder builder;

  builder.AddListeningPort(std::string(server_address),
                           grpc::InsecureServerCredentials());
  builder.RegisterService(&process_service_);

  server_ = builder.BuildAndStart();
};

void OrbitGrpcServerImpl::Shutdown() {
  server_->Shutdown();
}

void OrbitGrpcServerImpl::Wait() {
  server_->Wait();
}

}  // namespace

std::unique_ptr<OrbitGrpcServer> OrbitGrpcServer::Create(
    std::string_view server_address) {

  std::unique_ptr<OrbitGrpcServerImpl> server_impl =
      std::make_unique<OrbitGrpcServerImpl>();

  server_impl->Init(server_address);

  return std::move(server_impl);
}

