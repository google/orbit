#include <iostream>

#include <csignal>

#include "OrbitGrpcServer.h"
#include "OrbitService.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"

// TODO: Default it 127.0.0.1 once ssh tunneling is enabled.
ABSL_FLAG(std::string, grpc_server_address, "0.0.0.0:44755",
          "Grpc server address");

namespace {
std::atomic<bool> exit_requested;

void sigint_handler(int signum) {
  if (signum == SIGINT) {
    exit_requested = true;
  }
}

void install_sigint_handler() {
  struct sigaction act;
  act.sa_handler = sigint_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_restorer = nullptr;
  sigaction(SIGINT, &act, nullptr);
}
}  // namespace

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Orbit CPU Profiler Service");
  absl::ParseCommandLine(argc, argv);

  install_sigint_handler();

  std::unique_ptr<OrbitGrpcServer> grpc_server;
  std::string grpc_server_address = absl::GetFlag(FLAGS_grpc_server_address);
  std::cout << "Starting GRPC server at " << grpc_server_address << std::endl;
  grpc_server = OrbitGrpcServer::Create(grpc_server_address);

  std::cout << "Starting OrbitService" << std::endl;
  OrbitService service;
  exit_requested = false;
  service.Run(&exit_requested);
}
