// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/flags/usage_config.h>
#include <absl/strings/str_format.h>
#include <absl/strings/string_view.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitClientGgp/ClientGgp.h"
#include "OrbitClientGgp/ClientGgpOptions.h"
#include "OrbitVersion/OrbitVersion.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc service's port");
ABSL_FLAG(int32_t, pid, 0, "pid to capture");
ABSL_FLAG(uint32_t, capture_length, 10, "duration of capture in seconds");
ABSL_FLAG(std::vector<std::string>, functions, {},
          "Comma-separated list of functions to hook to the capture");
ABSL_FLAG(std::string, file_name, "", "File name used for saving the capture");
ABSL_FLAG(std::string, file_directory, "/var/game/",
          "Path to locate orbit file. By default it is /var/game/");
ABSL_FLAG(std::string, log_directory, "",
          "Path to locate debug file. By default only stdout is used for logs");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(uint16_t, stack_dump_size, 65000,
          "Number of bytes to copy from the stack per sample. Max: 65000");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");
ABSL_FLAG(bool, thread_state, false, "Collect thread states");
ABSL_FLAG(uint64_t, max_local_marker_depth_per_command_buffer, std::numeric_limits<uint64_t>::max(),
          "Max local marker depth per command buffer");

namespace {

std::string GetLogFilePath(std::string_view log_directory) {
  std::filesystem::path log_directory_path{log_directory};
  std::filesystem::create_directory(log_directory_path);
  std::filesystem::path log_file_path = log_directory_path / "OrbitClientGgp.log";
  ORBIT_LOG("Log file: %s", log_file_path);
  return log_file_path;
}

}  // namespace

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Orbit CPU Profiler Ggp Client");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &orbit_version::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  const std::string log_directory = absl::GetFlag(FLAGS_log_directory);
  if (!log_directory.empty()) {
    orbit_base::InitLogFile(GetLogFilePath(log_directory));
  }

  if (absl::GetFlag(FLAGS_pid) == 0) {
    ORBIT_FATAL("pid to capture not provided; set using -pid");
  }

  ClientGgpOptions options;
  uint64_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  options.grpc_server_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  options.capture_pid = absl::GetFlag(FLAGS_pid);
  options.capture_functions = absl::GetFlag(FLAGS_functions);
  options.capture_file_name = absl::GetFlag(FLAGS_file_name);
  options.capture_file_directory = absl::GetFlag(FLAGS_file_directory);
  options.samples_per_second = absl::GetFlag(FLAGS_sampling_rate);
  uint16_t stack_dump_size = absl::GetFlag(FLAGS_stack_dump_size);
  ORBIT_CHECK(stack_dump_size <= 65000);
  options.stack_dump_size = stack_dump_size;
  options.use_framepointer_unwinding = absl::GetFlag(FLAGS_frame_pointer_unwinding);

  ClientGgp client_ggp(std::move(options));
  if (!client_ggp.InitClient()) {
    return -1;
  }

  // The request is done in a separate thread to avoid blocking main()
  // It is needed to provide a thread pool
  std::shared_ptr<orbit_base::ThreadPool> thread_pool =
      orbit_base::ThreadPool::Create(1, 1, absl::Seconds(1));
  auto start_capture_result = client_ggp.RequestStartCapture(thread_pool.get());
  if (start_capture_result.has_error()) {
    thread_pool->ShutdownAndWait();
    ORBIT_FATAL("Unable to start capture: %s", start_capture_result.error().message());
  }

  // Captures for the period of time requested
  uint32_t capture_length = absl::GetFlag(FLAGS_capture_length);
  ORBIT_LOG("Go to sleep for %d seconds", capture_length);
  absl::SleepFor(absl::Seconds(capture_length));
  ORBIT_LOG("Back from sleep");

  // Requests to stop the capture and waits for thread to finish
  if (!client_ggp.StopCapture()) {
    thread_pool->ShutdownAndWait();
    ORBIT_FATAL("Unable to stop the capture; exiting");
  }

  ORBIT_LOG("Shut down the thread and wait for it to finish");
  thread_pool->ShutdownAndWait();

  ORBIT_LOG("All done");
  return 0;
}
