// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientGgp.h"

ClientGgp::ClientGgp(ClientGgpOptions&& options) 
    : options_(std::move(options)) {}

bool ClientGgp::InitClient() {
  if (options_.grpc_server_address.empty()){
    ERROR("gRPC server address not provided");
    return false;
  }

  // Create channel
  grpc::ChannelArguments channel_arguments;
  channel_arguments.SetMaxReceiveMessageSize(
      std::numeric_limits<int32_t>::max());

  grpc_channel_ = grpc::CreateCustomChannel(
      options_.grpc_server_address, 
      grpc::InsecureChannelCredentials(), 
      channel_arguments);

  if (!grpc_channel_) {
    ERROR("Unable to create GRPC channel to %s",
          options_.grpc_server_address);
    return false;
  }
  
  LOG("Created GRPC channel to %s", options_.grpc_server_address);

  // Instantiate CaptureClient
  //capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, nullptr);

  return true;
}