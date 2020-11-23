// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpService.h"
#include "OrbitVersion/OrbitVersion.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"

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

namespace {

std::string GetLogFilePath(const std::string& log_directory) {
  std::filesystem::path log_directory_path{log_directory};
  std::filesystem::create_directory(log_directory_path);
  const std::string log_file_path = log_directory_path / "OrbitCaptureGgpService.log";
  LOG("Log file: %s", log_file_path);
  return log_file_path;
}

}  // namespace

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Orbit CPU Profiler Ggp Client");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &OrbitCore::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  LOG("------------------------------------");
  LOG("OrbitCaptureGgpService started");
  LOG("------------------------------------");

  const std::string log_directory = absl::GetFlag(FLAGS_log_directory);
  if (!log_directory.empty()) {
    InitLogFile(GetLogFilePath(log_directory));
  }

  if (!absl::GetFlag(FLAGS_pid)) {
    FATAL("pid to capture not provided; set using -pid");
  }

  // Start the service and waits for calls from the game
  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  OrbitCaptureGgpService service(grpc_port);
  service.RunServer();

  return 0;
}
