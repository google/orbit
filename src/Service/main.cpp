// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdint.h>

#include <atomic>
#include <csignal>
#include <filesystem>
#include <string>

#include "OrbitBase/Logging.h"
#include "OrbitService.h"
#include "OrbitVersion/OrbitVersion.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"

ABSL_FLAG(uint64_t, grpc_port, 44765, "gRPC server port");

ABSL_FLAG(bool, devmode, false, "Enable developer mode");

int main(int argc, char** argv) {
  orbit_base::InitLogFile(orbit_service::OrbitService::GetLogFilePath());

  absl::SetProgramUsageMessage("Orbit CPU Profiler Service");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &orbit_version::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  bool dev_mode = absl::GetFlag(FLAGS_devmode);

  orbit_service::OrbitService service{grpc_port, dev_mode};
  return service.Run();
}
