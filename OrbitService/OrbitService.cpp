// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitService.h"

#include <fcntl.h>

#include "OrbitAsioServer.h"
#include "OrbitGrpcServer.h"

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  std::cout << "Starting GRPC server at " << grpc_address_ << std::endl;
  std::unique_ptr<OrbitGrpcServer> grpc_server;
  grpc_server = OrbitGrpcServer::Create(grpc_address_);

  std::cout << "Starting Asio server on port " << asio_port_ << std::endl;
  OrbitAsioServer asio_server{asio_port_, tracing_options_};

  // make stdin non blocking
  fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  // buffer for reading from stdin.
  char buffer[0x10] = "";

  // Main loop
  while (!(*exit_requested)) {
    // read from stdin to buffer. This detect the potentially sent EOF
    while (fgets(buffer, sizeof(buffer), stdin) != nullptr) continue;
    // exit if EOF occurred. This is used to shutdown via ssh.
    if (feof(stdin)) *exit_requested = true;

    asio_server.LoopTick();
    Sleep(16);
  }

  grpc_server->Shutdown();
  grpc_server->Wait();
}
