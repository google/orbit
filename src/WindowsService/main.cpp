// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <atomic>
#include <csignal>
#include <filesystem>
#include <string>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitService.h"
#include "OrbitVersion/OrbitVersion.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "gRPC server port");

ABSL_FLAG(bool, devmode, false, "Enable developer mode");

namespace {

std::atomic<bool> exit_requested;

std::string GetLogFilePath() {
  std::filesystem::path log_dir = orbit_base::GetExecutablePath() / "logs";
  std::error_code ec;
  std::filesystem::create_directory(log_dir, ec);
  const std::filesystem::path log_file_path = log_dir / "OrbitService.log";
  return log_file_path.string();
}

}  // namespace

int main(int argc, char** argv) {
  orbit_base::InitLogFile(GetLogFilePath());

  absl::SetProgramUsageMessage("Orbit CPU Profiler Service");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &orbit_version::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  bool dev_mode = absl::GetFlag(FLAGS_devmode);

  exit_requested = false;
  orbit_windows_service::OrbitService service{grpc_port, dev_mode};
  service.Run(&exit_requested);
}
