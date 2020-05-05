#ifndef ORBIT_SERVICE_ORBIT_SERVICE_H
#define ORBIT_SERVICE_ORBIT_SERVICE_H

#include <atomic>
#include <string>
#include <utility>

class OrbitService {
 public:
  OrbitService(std::string grpc_address, uint16_t asio_port)
      : grpc_address_{std::move(grpc_address)}, asio_port_{asio_port} {}

  void Run(std::atomic<bool>* exit_requested);

 private:
  std::string grpc_address_;
  uint16_t asio_port_;
};

#endif  // ORBIT_SERVICE_ORBIT_SERVICE_H
