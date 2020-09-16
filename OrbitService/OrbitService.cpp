// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitService.h"

#include <absl/strings/match.h>
#include <fcntl.h>

#include <chrono>
#include <cstdio>
#include <thread>

#include "OrbitBase/Logging.h"
#include "OrbitGrpcServer.h"
#include "OrbitVersion/OrbitVersion.h"

namespace {

static std::string ReadStdIn() {
  int tmp = fgetc(stdin);
  if (tmp == -1) return "";

  std::string result;
  do {
    result += static_cast<char>(tmp);
    tmp = fgetc(stdin);
  } while (tmp != -1);

  return result;
}

static bool IsSshConnectionAlive(
    std::chrono::time_point<std::chrono::steady_clock> last_ssh_message,
    const int timeout_in_seconds) {
  return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() -
                                                          last_ssh_message)
             .count() < timeout_in_seconds;
}
}  // namespace

namespace orbit_service {

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  LOG("Running Orbit Service version %s", OrbitCore::GetVersion());
  std::string grpc_address = absl::StrFormat("127.0.0.1:%d", grpc_port_);
  LOG("Starting gRPC server at %s", grpc_address);
  std::unique_ptr<OrbitGrpcServer> grpc_server;
  grpc_server = OrbitGrpcServer::Create(grpc_address);
  if (grpc_server == nullptr) {
    ERROR("Unable to start gRPC server");
    return;
  }
  LOG("gRPC server is running");

  // Make stdin non-blocking.
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

  // Wait for exit_request or for the watchdog to expire.
  while (!(*exit_requested)) {
    std::string stdin_data = ReadStdIn();
    // If ssh sends EOF, end main loop.
    if (feof(stdin) != 0) break;

    if (IsSshWatchdogActive() || absl::StrContains(stdin_data, kStartWatchdogPassphrase)) {
      if (!stdin_data.empty()) {
        last_stdin_message_ = std::chrono::steady_clock::now();
      }

      if (!IsSshConnectionAlive(last_stdin_message_.value(), kWatchdogTimeoutInSeconds)) {
        break;
      }
    }

    std::this_thread::sleep_for(std::chrono::seconds{1});
  }

  grpc_server->Shutdown();
  grpc_server->Wait();
}

}  // namespace orbit_service