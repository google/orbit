// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERIVICE_ORBIT_GRPC_SERVER_H_
#define ORBIT_SERIVICE_ORBIT_GRPC_SERVER_H_

#include <memory>
#include <string>


// Wrapper around GRPC server. This class takes care of regitering
// all GRPC services.
//
// Usage example:
//   auto server = OrbitGrpcServer::Create("localhost:44744");
//   server->Wait();
class OrbitGrpcServer {
 public:
  OrbitGrpcServer() = default;
  virtual ~OrbitGrpcServer() = default;

  // Proxy calls to grpc::Server::Wait() and grpc::Server::Shutdown
  virtual void Shutdown() = 0;
  virtual void Wait() = 0;

  // Creates a server listening specified address and registers all
  // necessary services.
  static std::unique_ptr<OrbitGrpcServer> Create(
      std::string_view server_address);
};

#endif  // ORBIT_SERIVICE_ORBIT_GRPC_SERVER_H_
