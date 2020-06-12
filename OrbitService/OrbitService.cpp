// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitService.h"

#include <absl/strings/match.h>
#include <fcntl.h>

#include <chrono>
#include <cstdio>

#include "OrbitAsioServer.h"
#include "OrbitBase/Logging.h"
#include "OrbitGrpcServer.h"

static std::string ReadStdIn() {
  char tmp = fgetc(stdin);
  if (tmp == -1) return "";

  std::string result;
  do {
    result += tmp;
    tmp = fgetc(stdin);
  } while (tmp != -1);

  return result;
}

static bool IsSshConnectionAlive(
    std::chrono::time_point<std::chrono::steady_clock> last_ssh_message,
    const int timeout_in_seconds) {
  return std::chrono::duration_cast<std::chrono::seconds>(
             std::chrono::steady_clock::now() - last_ssh_message)
             .count() < timeout_in_seconds;
}

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  std::string grpc_address = absl::StrFormat("127.0.0.1:%d", grpc_port_);
  LOG("Starting GRPC server at %s", grpc_address);
  std::unique_ptr<OrbitGrpcServer> grpc_server;
  grpc_server = OrbitGrpcServer::Create(grpc_address);

  LOG("Starting Asio server on port %i", asio_port_);
  OrbitAsioServer asio_server{asio_port_, tracing_options_};

  // make stdin non blocking
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);

  // Main loop
  while (!(*exit_requested)) {
    asio_server.LoopTick();

    std::string stdin_data = ReadStdIn();
    // if ssh sends EOF, end main loop
    if (feof(stdin)) break;

    if (IsSshWatchdogActive() ||
        absl::StrContains(stdin_data, start_passphrase_)) {
      if (!stdin_data.empty()) {
        last_stdin_message_ = std::chrono::steady_clock::now();
      }

      if (!IsSshConnectionAlive(last_stdin_message_.value(),
                                timeout_in_seconds_)) {
        break;
      }
    }

    Sleep(16);
  }

  grpc_server->Shutdown();
  grpc_server->Wait();
}
