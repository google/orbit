// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/strings/str_format.h>
#include <absl/strings/string_view.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"

ABSL_FLAG(uint16_t, grpc_port, 44767, "gRPC server port for capture ggp service");

// Created for testing purposes and to document the use of CaptureClientGgpClient, which is
// expected to be used in a game. Tests the available methods and it should be kept up to date when
// these are included, removed or updated in CaptureClientGgpClient
int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  uint64_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  std::string grpc_server_address = absl::StrFormat("127.0.0.1:%d", grpc_port);
  CaptureClientGgpClient ggp_capture_client = CaptureClientGgpClient(grpc_server_address);

  // Waits for input to run the grpc call requested by the user

  constexpr const int kStartCaptureCommand = 1;
  constexpr const int kStopAndSaveCaptureCommand = 2;
  constexpr const int kUpdateSelectedFunctionsCommand = 3;
  constexpr const int kShutdownServiceCommand = 4;
  bool exit = false;
  while (!exit) {
    int i;
    std::cout << "\n";
    std::cout << "List of available commands:\n";
    std::cout << "------------------------------\n";
    std::cout << kStartCaptureCommand << " Start capture\n";
    std::cout << kStopAndSaveCaptureCommand << " Stop and save capture\n";
    std::cout << kUpdateSelectedFunctionsCommand << " Hook functions\n";
    std::cout << kShutdownServiceCommand << " Shutdown service and exit\n";
    std::cout << "\n";
    std::cout << "Introduce your choice (" << kStartCaptureCommand << "-" << kShutdownServiceCommand
              << "): ";

    std::cin >> i;
    switch (i) {
      case kStartCaptureCommand:
        ORBIT_LOG("Chosen %d: Start capture", i);
        ggp_capture_client.StartCapture();
        break;
      case kStopAndSaveCaptureCommand:
        ORBIT_LOG("Chosen %d: Stop and save capture", i);
        ggp_capture_client.StopCapture();
        break;
      case kUpdateSelectedFunctionsCommand: {
        std::vector<std::string> selected_functions;
        std::string function;
        std::cout << "Introduce function to hook (Enter ! when you are done): ";
        std::cin >> function;
        while (function != "!") {
          selected_functions.push_back(function);
          std::cout << "Introduce function to hook (Enter ! when you are done): ";
          std::cin >> function;
        }
        ggp_capture_client.UpdateSelectedFunctions(selected_functions);
        break;
      }
      case kShutdownServiceCommand:
        ORBIT_LOG("Chosen %d: Shutdown service and exit", i);
        exit = true;
        break;
      default:
        ORBIT_ERROR("Option selected not valid. Try again");
    }
  }

  ggp_capture_client.ShutdownService();

  return 1;
}
