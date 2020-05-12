#ifndef ORBIT_SERVICE_ORBIT_SERVICE_H
#define ORBIT_SERVICE_ORBIT_SERVICE_H

#include <atomic>
#include <string>
#include <utility>

#include "OrbitLinuxTracing/TracingOptions.h"

class OrbitService {
 public:
  OrbitService(std::string grpc_address, uint16_t asio_port,
               LinuxTracing::SamplingMethod sampling_method)
      : grpc_address_{std::move(grpc_address)},
        asio_port_{asio_port},
        sampling_method_{sampling_method} {}

  void Run(std::atomic<bool>* exit_requested);

 private:
  std::string grpc_address_;
  uint16_t asio_port_;
  LinuxTracing::SamplingMethod sampling_method_;
};

#endif  // ORBIT_SERVICE_ORBIT_SERVICE_H
