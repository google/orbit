#include <csignal>
#include <iostream>

#include "OrbitLinuxTracing/TracingOptions.h"
#include "OrbitService.h"
#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"

ABSL_FLAG(uint16_t, asio_port, 44766, "Asio TCP server port");

ABSL_FLAG(uint64_t, grpc_port, 44765, "Grpc server port");

// TODO: Remove this flag once we have a ui option to specify.
ABSL_FLAG(bool, frame_pointer_unwinding, false,
          "Use frame pointers for unwinding");

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

  uint16_t grpc_port = absl::GetFlag(FLAGS_grpc_port);
  uint16_t asio_port = absl::GetFlag(FLAGS_asio_port);

  LinuxTracing::TracingOptions tracing_options;

  if (absl::GetFlag(FLAGS_frame_pointer_unwinding)) {
    tracing_options.sampling_method =
        LinuxTracing::SamplingMethod::kFramePointers;
  }

  exit_requested = false;
  OrbitService service{grpc_port, asio_port, tracing_options};
  service.Run(&exit_requested);
}
