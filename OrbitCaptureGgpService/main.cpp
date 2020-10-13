// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>
#include <memory>
#include <string>

#include "OrbitCaptureGgpService.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"

ABSL_FLAG(uint64_t, grpc_port, 44767, "gRPC server port");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);

  OrbitCaptureGgpService service{grpc_port};
  service.RunServer();

  return 0;
}
