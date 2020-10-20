// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpService.h"

#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpServiceImpl.h"
#include "absl/strings/str_cat.h"
#include "grpcpp/ext/proto_server_reflection_plugin.h"

using grpc::Server;
using grpc::ServerBuilder;

void OrbitCaptureGgpService::RunServer() {
  std::string server_address = absl::StrFormat("127.0.0.1:%d", grpc_port_);
  CaptureClientGgpServiceImpl ggp_capture_service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();

  LOG("Starting gRPC capture ggp server at %s", server_address);
  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&ggp_capture_service);
  std::unique_ptr<Server> server(builder.BuildAndStart());

  if (server == nullptr) {
    ERROR("Unable to start gRPC server");
    return;
  }

  std::thread ServerShutdownWatcher([&ggp_capture_service, &server]() {
    static constexpr auto WatcherFrequency = std::chrono::seconds(5);
    while (!ggp_capture_service.ShutdownRequested()) {
      std::this_thread::sleep_for(WatcherFrequency);
    }
    server->Shutdown();
  });

  LOG("Capture ggp server listening on %s", server_address);
  server->Wait();
  ServerShutdownWatcher.join();
}
