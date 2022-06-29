// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/flags/usage.h>
#include <absl/flags/usage_config.h>
#include <stdint.h>

#include <atomic>
#include <csignal>
#include <filesystem>
#include <string>

#ifdef _WIN32
#include <Windows.h>
#endif

#include "OrbitBase/Logging.h"
#include "OrbitService.h"
#include "OrbitVersion/OrbitVersion.h"

#ifdef _WIN32
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/GetLastError.h"
#endif

ABSL_FLAG(uint64_t, grpc_port, 44765, "gRPC server port");

ABSL_FLAG(bool, producer_side_server, true,
          "Start the producer-side server: set it to false if you need to start a second instance "
          "of OrbitService");

ABSL_FLAG(bool, devmode, false, "Enable developer mode");

namespace {

std::atomic<bool> exit_requested;

#ifdef WIN32

BOOL WINAPI CtrlHandler(DWORD event_type) {
  // Handle the CTRL-C signal.
  if (event_type == CTRL_C_EVENT || event_type == CTRL_CLOSE_EVENT) {
    exit_requested = true;
    return TRUE;
  }

  // Pass other signals to the next handler.
  return FALSE;
}

void InstallSigintHandler() {
  if (!SetConsoleCtrlHandler(CtrlHandler, TRUE)) {
    ORBIT_ERROR("%s", orbit_base::GetLastErrorAsString("SetConsoleCtrlHandler"));
  }
}

#else

void SigintHandler(int signum) {
  if (signum == SIGINT) {
    exit_requested = true;
  }
}

void InstallSigintHandler() {
  struct sigaction act {};
  act.sa_handler = SigintHandler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_restorer = nullptr;
  sigaction(SIGINT, &act, nullptr);
}

#endif

std::filesystem::path GetLogFilePath() {
#ifdef WIN32
  std::filesystem::path log_dir = orbit_base::GetExecutablePath() / "logs";
#else
  std::filesystem::path log_dir = "/var/log";
#endif
  std::error_code ec;
  std::filesystem::create_directory(log_dir, ec);
  return log_dir / "OrbitService.log";
}

}  // namespace

int main(int argc, char** argv) {
  orbit_base::InitLogFile(GetLogFilePath());

  absl::SetProgramUsageMessage("Orbit CPU Profiler Service");
  absl::SetFlagsUsageConfig(absl::FlagsUsageConfig{{}, {}, {}, &orbit_version::GetBuildReport, {}});
  absl::ParseCommandLine(argc, argv);

  InstallSigintHandler();

  const uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  const bool start_producer_side_server = absl::GetFlag(FLAGS_producer_side_server);
  const bool dev_mode = absl::GetFlag(FLAGS_devmode);

  exit_requested = false;
  orbit_service::OrbitService service{grpc_port, start_producer_side_server, dev_mode};
  auto result = service.Run(&exit_requested);

  if (!result.has_error()) return 0;

  ORBIT_ERROR("%s", result.error().message());
  std::puts(result.error().message().c_str());
  std::ignore = std::fflush(stdout);

  // Note that changing the exit code value will affect the UI client which is checking for the
  // exact value.
  constexpr int kExitCodeIndicatingErrorMessage = 42;
  return kExitCodeIndicatingErrorMessage;
}
