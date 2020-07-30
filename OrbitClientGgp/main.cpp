// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "absl/strings/str_format.h"

#include "grpcpp/grpcpp.h"

#include "ClientGgpOptions.h"
#include "ClientGgp.h"
#include "OrbitBase/Logging.h"
#include "OrbitCaptureClient/CaptureClient.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");


int main() {
  // TODO This must be in client class but here in the meantime
    uint16_t grpc_port_ = absl::GetFlag(FLAGS_grpc_port);

  ClientGgpOptions options;
  options.grpc_server_address = 
    absl::StrFormat("127.0.0.1:%d", grpc_port_);

  // TODO: move to ClientGgp / delete following line
  std::string grpc_server_address = options.grpc_server_address;

  // Creating channel
  grpc::ChannelArguments channel_arguments;
  channel_arguments.SetMaxReceiveMessageSize(
      std::numeric_limits<int32_t>::max());

  std::shared_ptr<grpc::Channel> grpc_channel_ = grpc::CreateCustomChannel(
      grpc_server_address, grpc::InsecureChannelCredentials(),
      channel_arguments);

  if (!grpc_channel_) {
    ERROR("Unable to create GRPC channel to %s",
          grpc_server_address);
  } else {
    LOG("Created GRPC channel to %s", grpc_server_address);
}

  // Stubs and details of the service is done in their classes
  
  std::unique_ptr<CaptureClient> capture_client_ = std::make_unique<CaptureClient>(grpc_channel_, nullptr);
  return 0;
}