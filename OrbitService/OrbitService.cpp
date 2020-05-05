#include "OrbitService.h"

#include "OrbitAsioServer.h"
#include "OrbitGrpcServer.h"

void OrbitService::Run(std::atomic<bool>* exit_requested) {
  std::cout << "Starting GRPC server at " << grpc_address_ << std::endl;
  std::unique_ptr<OrbitGrpcServer> grpc_server;
  grpc_server = OrbitGrpcServer::Create(grpc_address_);

  std::cout << "Starting Asio server on port " << asio_port_ << std::endl;
  OrbitAsioServer asio_server{asio_port_};
  asio_server.Run(exit_requested);

  grpc_server->Shutdown();
  grpc_server->Wait();
}
