// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGrpcServer.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/security/server_credentials.h>

#include <string>
#include <utility>

#include "CaptureServiceBase/CaptureStartStopListener.h"

#ifdef __linux

#include "CrashService/CrashServiceImpl.h"
#include "FramePointerValidatorService/FramePointerValidatorServiceImpl.h"
#include "LinuxCaptureService/LinuxCaptureService.h"
#include "ProcessService/ProcessServiceImpl.h"
#include "TracepointService/TracepointServiceImpl.h"

#else

#include "WindowsCaptureService/WindowsCaptureService.h"
#include "WindowsProcessService/ProcessServiceImpl.h"

#endif

namespace orbit_service {

namespace {

class OrbitGrpcServerImpl final : public OrbitGrpcServer {
 public:
  OrbitGrpcServerImpl() = default;
  OrbitGrpcServerImpl(const OrbitGrpcServerImpl&) = delete;
  OrbitGrpcServerImpl& operator=(OrbitGrpcServerImpl&) = delete;

  [[nodiscard]] bool Init(std::string_view server_address, bool dev_mode);

  void Shutdown() override;
  void Wait() override;

  void AddCaptureStartStopListener(
      orbit_capture_service_base::CaptureStartStopListener* listener) override;
  void RemoveCaptureStartStopListener(
      orbit_capture_service_base::CaptureStartStopListener* listener) override;

 private:
#ifdef __linux
  orbit_linux_capture_service::LinuxCaptureService capture_service_;
  orbit_process_service::ProcessServiceImpl process_service_;
  orbit_tracepoint_service::TracepointServiceImpl tracepoint_service_;
  orbit_frame_pointer_validator_service::FramePointerValidatorServiceImpl
      frame_pointer_validator_service_;
  orbit_crash_service::CrashServiceImpl crash_service_;
#else
  orbit_windows_capture_service::WindowsCaptureService capture_service_;
  orbit_windows_process_service::ProcessServiceImpl process_service_;
#endif

  std::unique_ptr<grpc::Server> server_;
};

bool OrbitGrpcServerImpl::Init(std::string_view server_address, bool dev_mode) {
  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  grpc::ServerBuilder builder;

  // Increase maximum receive size for unbounded "CaptureOptions" message.
  builder.SetMaxReceiveMessageSize(std::numeric_limits<int32_t>::max());

  builder.AddListeningPort(std::string(server_address), grpc::InsecureServerCredentials());
  builder.RegisterService(&capture_service_);
  builder.RegisterService(&process_service_);

#ifdef __linux
  builder.RegisterService(&tracepoint_service_);
  builder.RegisterService(&frame_pointer_validator_service_);
  if (dev_mode) {
    builder.RegisterService(&crash_service_);
  }
#endif

  server_ = builder.BuildAndStart();

  return server_ != nullptr;
}

void OrbitGrpcServerImpl::Shutdown() { server_->Shutdown(); }

void OrbitGrpcServerImpl::Wait() { server_->Wait(); }

void OrbitGrpcServerImpl::AddCaptureStartStopListener(
    orbit_capture_service_base::CaptureStartStopListener* listener) {
  capture_service_.AddCaptureStartStopListener(listener);
}

void OrbitGrpcServerImpl::RemoveCaptureStartStopListener(
    orbit_capture_service_base::CaptureStartStopListener* listener) {
  capture_service_.RemoveCaptureStartStopListener(listener);
}

}  // namespace

std::unique_ptr<OrbitGrpcServer> OrbitGrpcServer::Create(std::string_view server_address,
                                                         bool dev_mode) {
  std::unique_ptr<OrbitGrpcServerImpl> server_impl = std::make_unique<OrbitGrpcServerImpl>();

  if (!server_impl->Init(server_address, dev_mode)) {
    return nullptr;
  }

  return std::move(server_impl);
}

}  // namespace orbit_service
