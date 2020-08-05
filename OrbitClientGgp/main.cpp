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

#include "ClientGgp.h"
#include "OrbitBase/Logging.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");

int main() {
  // TODO This must be in client class but here in the meantime
    uint16_t grpc_port_ = absl::GetFlag(FLAGS_grpc_port);

  ClientGgpOptions options;
  options.grpc_server_address = 
    absl::StrFormat("127.0.0.1:%d", grpc_port_);

  ClientGgp client_ggp(std::move(options));
  if (!client_ggp.InitClient()){
    return -1;
  }
  
  return 0;
}