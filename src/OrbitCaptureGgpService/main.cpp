// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/flags/usage_config.h>
#include <absl/strings/string_view.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpService.h"
#include "OrbitVersion/OrbitVersion.h"

ABSL_FLAG(uint16_t, grpc_port, 44767, "gRPC server port for capture ggp service");
ABSL_FLAG(uint16_t, orbit_service_grpc_port, 44765, "gRPC server port for OrbitService");
ABSL_FLAG(int32_t, pid, 0, "pid to capture");
ABSL_FLAG(std::vector<std::string>, functions, {},
          "Comma-separated list of functions to hook to the capture");
ABSL_FLAG(std::string, file_name, "", "File name used for saving the capture");
ABSL_FLAG(std::string, file_directory, "/var/game/",
          "Path to locate .orbit file. By default it is /var/game/");
ABSL_FLAG(std::string, log_directory, "",
          "Path to locate debug file. By default only stdout is used for logs");
ABSL_FLAG(uint16_t, sampling_rate, 1000, "Frequency of callstack sampling in samples per second");
ABSL_FLAG(bool, frame_pointer_unwinding, false, "Use frame pointers for unwinding");
ABSL_FLAG(bool, thread_state, false, "Collect thread states");
ABSL_FLAG(uint64_t, max_local_marker_depth_per_command_buffer, std::numeric_limits<uint64_t>::max(),
          "Max local marker depth per command buffer");

namespace {

std::string GetLogFilePath(std::string_view log_directory) {
  std::filesystem::path log_directory_path{log_directory};
  std::filesystem::create_directory(log_directory_path);
  const std::string log_file_path = log_directory_path / "OrbitCaptureGgpService.log";
  ORBIT_LOG("Log file: %s", log_file_path);
  return log_file_path;
}

}  // namespace

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Orbit CPU Profiler Ggp Client");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &orbit_version::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  ORBIT_LOG("------------------------------------");
  ORBIT_LOG("OrbitCaptureGgpService started");
  ORBIT_LOG("------------------------------------");

  const std::string log_directory = absl::GetFlag(FLAGS_log_directory);
  if (!log_directory.empty()) {
    orbit_base::InitLogFile(GetLogFilePath(log_directory));
  }

  if (absl::GetFlag(FLAGS_pid) == 0) {
    ORBIT_FATAL("pid to capture not provided; set using -pid");
  }

  // Start the service and waits for calls from the game
  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  OrbitCaptureGgpService service(grpc_port);
  service.RunServer();

  return 0;
}
