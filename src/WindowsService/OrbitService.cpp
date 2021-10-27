// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitService.h"

#include <absl/strings/str_format.h>
#include <fcntl.h>

#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <filesystem>
#include <memory>
#include <string>
#include <thread>

#include "OrbitBase/ExecuteCommand.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitGrpcServer.h"
#include "OrbitVersion/OrbitVersion.h"

namespace orbit_windows_service {

// We try to determine the clock resolution and print out the determined value for
// postmortem debugging purposes. The resolution should be fairly small (in my tests
// it was ~35 nanoseconds).
static void PrintClockResolution() {
  LOG("%s", absl::StrFormat("Clock resolution: %d (ns)", orbit_base::EstimateClockResolution()));
}

static std::unique_ptr<OrbitGrpcServer> CreateGrpcServer(uint16_t grpc_port) {
  std::string grpc_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  LOG("Starting gRPC server at %s", grpc_address);
  std::unique_ptr<OrbitGrpcServer> grpc_server = OrbitGrpcServer::Create(grpc_address);
  if (grpc_server == nullptr) {
    ERROR("Unable to start gRPC server");
    return nullptr;
  }
  LOG("gRPC server is running");
  return grpc_server;
}

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  LOG("Running Orbit Service version %s", orbit_version::GetVersionString());
#ifndef NDEBUG
  LOG("**********************************");
  LOG("Orbit Service is running in DEBUG!");
  LOG("**********************************");
#endif

  PrintClockResolution();

  std::unique_ptr<OrbitGrpcServer> grpc_server = CreateGrpcServer(grpc_port_);
  if (grpc_server == nullptr) {
    return;
  }

  // Wait for exit_request.
  while (!(*exit_requested)) {
    std::this_thread::sleep_for(std::chrono::seconds{1});
  }

  grpc_server->Shutdown();
  grpc_server->Wait();
}

}  // namespace orbit_windows_service
