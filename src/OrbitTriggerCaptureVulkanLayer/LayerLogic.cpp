// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LayerLogic.h"

#include <unistd.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <ratio>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"
#include "absl/strings/str_format.h"

namespace {
constexpr const int kCaptureClientResultSuccess = 1;
constexpr uint16_t kGrpcPort = 44767;
}  // namespace

void LayerLogic::StartOrbitCaptureService() {
  pid_t pid = fork();
  if (pid < 0) {
    ORBIT_ERROR("Fork failed; not able to start Orbit capture service");
  } else if (pid == 0) {
    ORBIT_LOG("Starting Orbit capture service");
    std::string game_pid_str = absl::StrFormat("%d", getppid());
    std::vector<std::string> argv_str = layer_options_.BuildOrbitCaptureServiceArgv(game_pid_str);

    // Build argv in the format execv expects
    std::vector<char*> argv;
    for (auto const& arg_str : argv_str) {
      argv.push_back(const_cast<char*>(arg_str.c_str()));
    }
    // Execv requires NULL as the last argument
    argv.push_back(nullptr);

    std::string log_message = "Executing";
    for (auto const& arg : argv) {
      absl::StrAppendFormat(&log_message, " %s", arg);
    }
    ORBIT_LOG("%s", log_message);
    ORBIT_LOG("%d arguments", argv.size());

    execv(argv[0], argv.data());
  }
}

void LayerLogic::Init() {
  // Although this method is expected to be called just once, we include a flag to make sure the
  // gRPC service and client are not initialized more than once.
  if (!data_initialized_) {
    ORBIT_LOG("Making initializations required in the layer");

    // Initialize and load data from config file
    layer_options_.Init();

    // Start the orbit capture service in a new process.
    StartOrbitCaptureService();

    // Initialize the client and establish the channel to make calls to the service.
    std::string grpc_server_address = absl::StrFormat("127.0.0.1:%d", kGrpcPort);
    ggp_capture_client_ =
        std::unique_ptr<CaptureClientGgpClient>(new CaptureClientGgpClient(grpc_server_address));

    data_initialized_ = true;
  }
}

void LayerLogic::Destroy() {
  if (data_initialized_) {
    ggp_capture_client_->ShutdownService();
    data_initialized_ = false;
    orbit_capture_running_ = false;
    skip_logic_call_ = true;
  }
}

// QueuePresentKHR is called once per frame so we can calculate the time per frame. When this value
// is higher than a certain threshold, an Orbit capture is started and runs during a certain period
// of time; after which is stopped and saved.
void LayerLogic::ProcessQueuePresentKHR() {
  std::chrono::steady_clock::time_point current_time = std::chrono::steady_clock::now();
  // Ignore logic on the first call because times are not initialized. Also skipped right after a
  // capture has been stopped
  if (skip_logic_call_) {
    skip_logic_call_ = false;
    last_frame_time_ = current_time;
    return;
  }

  if (!orbit_capture_running_) {
    auto frame_time = std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(
        current_time - last_frame_time_);
    if (std::isgreater(frame_time.count(), layer_options_.GetFrameTimeThresholdMilliseconds())) {
      ORBIT_LOG("Time frame is %fms and exceeds the %fms threshold; starting capture",
                frame_time.count(), layer_options_.GetFrameTimeThresholdMilliseconds());
      RunCapture();
    }
  } else {
    // Stop capture if it has been running long enough
    auto capture_time = std::chrono::duration_cast<std::chrono::duration<int64_t>>(
        current_time - capture_started_time_);
    if (capture_time.count() >= layer_options_.GetCaptureLengthSeconds()) {
      ORBIT_LOG("Capture has been running for %ds; stopping it",
                layer_options_.GetCaptureLengthSeconds());
      StopCapture();
    }
  }

  last_frame_time_ = current_time;
}

void LayerLogic::RunCapture() {
  int capture_started = ggp_capture_client_->StartCapture();
  if (capture_started == kCaptureClientResultSuccess) {
    capture_started_time_ = std::chrono::steady_clock::now();
    orbit_capture_running_ = true;
  }
}

void LayerLogic::StopCapture() {
  int capture_stopped = ggp_capture_client_->StopCapture();
  if (capture_stopped == kCaptureClientResultSuccess) {
    orbit_capture_running_ = false;
    // The frame time is expected to be longer the next call so we skip the check
    skip_logic_call_ = true;
  }
}
