// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpService.h"

#include <absl/strings/str_format.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>
#include <grpcpp/security/server_credentials.h>

#include <chrono>
#include <memory>
#include <string>
#include <thread>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpServiceImpl.h"

using grpc::Server;
using grpc::ServerBuilder;

void OrbitCaptureGgpService::RunServer() const {
  std::string server_address = absl::StrFormat("127.0.0.1:%d", grpc_port_);
  CaptureClientGgpServiceImpl ggp_capture_service;

  grpc::EnableDefaultHealthCheckService(true);

  ORBIT_LOG("Starting gRPC capture ggp server at %s", server_address);
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&ggp_capture_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  if (server == nullptr) {
    ORBIT_ERROR("Unable to start gRPC server");
    return;
  }

  std::thread server_shutdown_watcher([&ggp_capture_service, &server]() {
    static constexpr auto kWatcherTickDuration = std::chrono::seconds(5);
    while (!ggp_capture_service.ShutdownFinished()) {
      std::this_thread::sleep_for(kWatcherTickDuration);
    }
    server->Shutdown();
  });

  ORBIT_LOG("Capture ggp server listening on %s", server_address);
  server->Wait();
  server_shutdown_watcher.join();
}
