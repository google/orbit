// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "ClientGgp.h"
#include "OrbitBase/ThreadPool.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");
ABSL_FLAG(int32_t, pid, 0, "pid to capture");
ABSL_FLAG(uint32_t, capture_length, 10, "duration of capture in seconds");
ABSL_FLAG(std::vector<std::string>, functions, {},
          "Comma-separated list of functions to hook to the capture");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  if (!absl::GetFlag(FLAGS_pid)) {
    FATAL("pid to capture not provided; set using -pid");
  }

  ClientGgpOptions options;
  uint64_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  options.grpc_server_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  options.capture_pid = absl::GetFlag(FLAGS_pid);
  options.capture_functions = absl::GetFlag(FLAGS_functions);

  ClientGgp client_ggp(std::move(options));
  if (!client_ggp.InitClient()) {
    return -1;
  }

  // The request is done in a separate thread to avoid blocking main()
  // It is needed to provide a thread pool
  std::unique_ptr<ThreadPool> thread_pool = ThreadPool::Create(1, 1, absl::Seconds(1));
  if (!client_ggp.RequestStartCapture(thread_pool.get())) {
    thread_pool->ShutdownAndWait();
    FATAL("Not possible to start the capture; exiting program");
  }

  // Captures for the period of time requested
  uint32_t capture_length = absl::GetFlag(FLAGS_capture_length);
  LOG("Go to sleep for %d seconds", capture_length);
  absl::SleepFor(absl::Seconds(capture_length));
  LOG("Back from sleep");

  // Requests to stop the capture and waits for thread to finish
  if (!client_ggp.StopCapture()) {
    thread_pool->ShutdownAndWait();
    FATAL("Not possible to stop the capture; exiting program");
  }
  LOG("Shut down the thread and wait for it to finish");
  thread_pool->ShutdownAndWait();

  // TODO: process/save capture data

  LOG("All done");
  return 0;
}