#ifndef ORBIT_SERVICE_ORBIT_SERVICE_H
#define ORBIT_SERVICE_ORBIT_SERVICE_H

#include <atomic>
#include <chrono>
#include <optional>
#include <string>
#include <utility>

#include "OrbitLinuxTracing/TracingOptions.h"

class OrbitService {
 public:
  OrbitService(std::string grpc_address, uint16_t asio_port,
               LinuxTracing::TracingOptions tracing_options)
      : grpc_address_{std::move(grpc_address)},
        asio_port_{asio_port},
        tracing_options_{tracing_options} {}

  void Run(std::atomic<bool>* exit_requested);

 private:
  bool IsSshWatchdogActive() { return last_stdin_message_ != std::nullopt; }

  std::string grpc_address_;
  uint16_t asio_port_;
  LinuxTracing::TracingOptions tracing_options_;

  std::optional<std::chrono::time_point<std::chrono::steady_clock>>
      last_stdin_message_ = std::nullopt;
  const std::string_view start_passphrase_ = "start_watchdog";
  const int timeout_in_seconds_ = 10;
};

#endif  // ORBIT_SERVICE_ORBIT_SERVICE_H
