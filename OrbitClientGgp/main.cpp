// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include "ClientGgp.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");
ABSL_FLAG(int32_t, pid, 0, "pid to capture");
ABSL_FLAG(uint32_t, capture_length, 10, "duration of capture in seconds");
ABSL_FLAG(uint16_t, sampling_rate, 1000,
          "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false,
          "Use frame pointers for unwinding");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  uint64_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  uint32_t capture_length = absl::GetFlag(FLAGS_capture_length);
  
  if (!absl::GetFlag(FLAGS_pid)) {
    FATAL("pid to capture not provided; set using -pid");
  }

  ClientGgpOptions options;
  options.grpc_server_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  options.capture_pid = absl::GetFlag(FLAGS_pid);

  ClientGgp client_ggp(std::move(options));
  if (!client_ggp.InitClient()){
    return -1;
  }

  LOG("Let's start the capture");

  if(!client_ggp.PrepareStartCapture()){
    return -1;
  }

  std::thread client_thread;

  // The request is done in a separate thread to avoid blocking main()
  client_thread = std::thread([&]() {
    client_ggp.RequestStartCapture();
  });

  LOG("Go to sleep");
  // Captures for the period of time requested
  absl::SleepFor(absl::Seconds(capture_length));
  LOG("Back from sleep");

  client_ggp.StopCapture();

  LOG("Shut down the thread and wait for it to finish");
  client_thread.join();

  //TODO: process/save capture data
  
  LOG("All done");
  return 0;
}