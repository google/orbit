// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <grpcpp/grpcpp.h>

#include <memory>
#include <string>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitCaptureGgpClient/OrbitCaptureGgpClient.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/flags/usage_config.h"
#include "services_ggp.grpc.pb.h"

ABSL_FLAG(uint16_t, grpc_port, 44767, "gRPC server port for capture ggp service");

// Created for testing purposes and to document the use of CaptureClientGgpClient, which is
// expected to be used in a game. Tests the available methods and it should be kept up to date when
// these are included, removed or updated in CaptureClientGgpClient
int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);

  // Create channel
  grpc::ChannelArguments channel_arguments;
  channel_arguments.SetMaxReceiveMessageSize(std::numeric_limits<int32_t>::max());
  uint64_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  std::string grpc_server_address = absl::StrFormat("127.0.0.1:%d", grpc_port);

  std::shared_ptr<::grpc::Channel> grpc_channel = grpc::CreateCustomChannel(
      grpc_server_address, grpc::InsecureChannelCredentials(), channel_arguments);
  if (!grpc_channel) {
    ERROR("Unable to create GRPC channel to %s", grpc_server_address);
    return 0;
  }
  LOG("Created GRPC channel to %s", grpc_server_address);

  CaptureClientGgpClient ggp_capture_client = CaptureClientGgpClient(grpc_channel);

  // Waits for input to run the grpc call requested by the user

  ErrorMessageOr<void> result = ErrorMessage();
  bool exit = false;
  while (!exit) {
    int i;
    std::cout << "\n";
    std::cout << "List of available commands:\n";
    std::cout << "------------------------------\n";
    std::cout << "1 Start capture\n";
    std::cout << "2 Stop and save capture\n";
    std::cout << "3 Hook functions\n";
    std::cout << "4 Shutdown service and exit\n";
    std::cout << "\n";
    std::cout << "Introduce your choice (1-4): ";

    std::cin >> i;
    switch (i) {
      case 1:
        LOG("Chosen %d: Start capture", i);
        result = ggp_capture_client.StartCapture();
        if (result.has_error()) {
          ERROR("Not possible to start capture: %s", result.error().message());
        }
        break;
      case 2:
        LOG("Chosen %d: Stop and save capture", i);
        result = ggp_capture_client.StopAndSaveCapture();
        if (result.has_error()) {
          ERROR("Not possible to stop or save capture: %s", result.error().message());
        }
        break;
      case 3: {
        std::vector<std::string> selected_functions;
        std::string function;
        std::cout << "Introduce function to hook (Enter ! when you are done): ";
        std::cin >> function;
        while (function != "!") {
          selected_functions.push_back(function);
          std::cout << "Introduce function to hook (Enter ! when you are done): ";
          std::cin >> function;
        }
        result = ggp_capture_client.UpdateSelectedFunctions(selected_functions);
        if (result.has_error()) {
          ERROR("Not possible to update functions %s", result.error().message());
        }
        break;
      }
      case 4:
        LOG("Chosen %d: Shutdown service and exit", i);
        exit = true;
        break;
      default:
        ERROR("Option selected not valid. Try again");
    }
  }

  ggp_capture_client.ShutdownService();

  return 1;
}
