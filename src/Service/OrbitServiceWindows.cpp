// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitService.h"

#include <absl/strings/str_format.h>
#include <stdio.h>
#include <windows.h>

#include <chrono>
#include <filesystem>
#include <memory>
#include <thread>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitGrpcServer.h"
#include "OrbitVersion/OrbitVersion.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "ProducerSideServer.h"

namespace orbit_service {

namespace {

std::unique_ptr<OrbitGrpcServer> CreateGrpcServer(uint16_t grpc_port, bool dev_mode) {
  std::string grpc_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  LOG("Starting gRPC server at %s", grpc_address);
  std::unique_ptr<OrbitGrpcServer> grpc_server = OrbitGrpcServer::Create(grpc_address, dev_mode);
  if (grpc_server == nullptr) {
    ERROR("Unable to start gRPC server");
    return nullptr;
  }
  LOG("gRPC server is running");
  return grpc_server;
}

std::unique_ptr<ProducerSideServer> BuildAndStartProducerSideServer(std::string uri) {
  auto producer_side_server = std::make_unique<ProducerSideServer>();
  LOG("Starting producer-side server at %s", uri);
  if (!producer_side_server->BuildAndStart(uri)) {
    ERROR("Unable to start producer-side server");
    return nullptr;
  }
  LOG("Producer-side server is running");
  return producer_side_server;
}

}  // namespace

std::string OrbitService::GetLogFilePath() {
  std::filesystem::path log_dir = orbit_base::GetExecutablePath() / "logs";
  std::error_code ec;
  std::filesystem::create_directory(log_dir, ec);
  const std::filesystem::path log_file_path = log_dir / "OrbitService.log";
  return log_file_path.string();
}

int OrbitService::Run(std::atomic<bool>* exit_requested) {
  LOG("Running Orbit Service version %s", orbit_version::GetVersionString());
#ifndef NDEBUG
  LOG("**********************************");
  LOG("Orbit Service is running in DEBUG!");
  LOG("**********************************");
#endif

  std::unique_ptr<OrbitGrpcServer> grpc_server = CreateGrpcServer(grpc_port_, dev_mode_);
  if (grpc_server == nullptr) {
    ERROR("Unable to create gRPC server.");
    return -1;
  }

  constexpr const char* kUri = "127.0.0.1:1789";
  std::unique_ptr<ProducerSideServer> producer_side_server = BuildAndStartProducerSideServer(kUri);
  if (producer_side_server == nullptr) {
    ERROR("Unable to build and start ProducerSideServer.");
    return -1;
  }
  grpc_server->AddCaptureStartStopListener(producer_side_server.get());

  // Wait for exit_request.
  while (!(*exit_requested)) {
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }

  producer_side_server->ShutdownAndWait();
  grpc_server->RemoveCaptureStartStopListener(producer_side_server.get());

  grpc_server->Shutdown();
  grpc_server->Wait();
  return 0;
}

}  // namespace orbit_service
