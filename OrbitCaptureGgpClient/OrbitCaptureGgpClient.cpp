// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"

#include <grpcpp/grpcpp.h>

#include <string>

#include "services_ggp.grpc.pb.h"

using orbit_grpc_protos::HelloReply;
using orbit_grpc_protos::HelloRequest;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

CaptureClientGgpClient::CaptureClientGgpClient(std::shared_ptr<Channel> channel)
    : stub_(orbit_grpc_protos::CaptureClientGgpService::NewStub(channel)) {}

// Assembles the client's payload, sends it and presents the response back
// from the server.
std::string CaptureClientGgpClient::SayHello(const std::string& user) {
  // Data we are sending to the server.
  HelloRequest request;
  request.set_name(user);

  // Container for the data we expect from the server.
  HelloReply reply;

  // Context for the client. It could be used to convey extra information to
  // the server and/or tweak certain RPC behaviors.
  ClientContext context;

  // The actual RPC.
  Status status = stub_->SayHello(&context, request, &reply);

  // Act upon its status.
  if (status.ok()) {
    return reply.message();
  } else {
    std::cout << status.error_code() << ": " << status.error_message() << std::endl;
    return "RPC failed";
  }
}