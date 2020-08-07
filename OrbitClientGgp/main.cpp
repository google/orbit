// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

//#include <iostream>
//#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
//#include "absl/flags/usage.h"
//#include "absl/flags/usage_config.h"
//#include "absl/strings/str_format.h"

#include "ClientGgp.h"
//#include "OrbitBase/Logging.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");
ABSL_FLAG(int32_t, pid, 0, "pid to capture");
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false,
          "Use frame pointers for unwinding");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  uint64_t grpc_port_ = absl::GetFlag(FLAGS_grpc_port);
  
  if (!absl::GetFlag(FLAGS_pid)) {
    ERROR("pid to capture not provided; set using -pid");
    return -1;
  }

  ClientGgpOptions options;
  options.grpc_server_address = 
        absl::StrFormat("127.0.0.1:%d", grpc_port_);
  options.capture_pid = absl::GetFlag(FLAGS_pid);

  //  std::unique_ptr<ClientGgp> client_ggp = std::make_unique<ClientGgp>(std::move(options));
  ClientGgp client_ggp(std::move(options));
  if (!client_ggp.InitClient()){
    return -1;
  }

  LOG("Lets start the capture");

  if(!client_ggp.PrepareStartCapture()){
    return -1;
  }

  // TODO: Do this in a new thread
  client_ggp.RequestStartCapture();

  // Captures for a little while
  sleep(10);

  client_ggp.StopCapture();

  return 0;
}