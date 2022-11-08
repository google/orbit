// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef _WIN32
#include <sys/socket.h>
#include <sys/un.h>
#endif

#include <absl/strings/match.h>
#include <absl/strings/str_format.h>

#include <filesystem>
#include <memory>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "ProducerSideService/BuildAndStartProducerSideServer.h"
#include "ProducerSideService/ProducerSideServer.h"

namespace orbit_producer_side_service {

// Tries to connect to the unix socket associated with the given path. If the connection succeeds we
// know that something is already listening there.
static ErrorMessageOr<void> VerifySocketAvailability(std::string_view socket_path) {
#ifndef _WIN32
  int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  sockaddr_un addr{};
  std::memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  socket_path.copy(addr.sun_path, sizeof(addr.sun_path) - 1);

  if (connect(socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
    close(socket_fd);

    return ErrorMessage{"OrbitService is already running on the machine."};
  }
#endif

  return outcome::success();
}

ErrorMessageOr<std::unique_ptr<ProducerSideServer>> BuildAndStartProducerSideServer(
    std::string_view server_address) {
  constexpr std::string_view kUnixDomainSocketPrefix = "unix:";
  const bool is_unix_domain_socket_connection =
      absl::StartsWith(server_address, kUnixDomainSocketPrefix);

  if (is_unix_domain_socket_connection) {
    std::string_view socket_path = server_address.substr(kUnixDomainSocketPrefix.length());
    const std::filesystem::path unix_domain_socket_dir =
        std::filesystem::path{socket_path}.parent_path();
    std::error_code error_code;
    std::filesystem::create_directories(unix_domain_socket_dir, error_code);
    if (error_code) {
      return ErrorMessage{
          absl::StrFormat("Unable to create directory for socket for producer-side server: %s",
                          error_code.message())};
    }

    // gRPC won't tell us whether the socket is already in use. Instead it will delete the inode and
    // create its own. So we fall back to checking whether we can connect to the socket here
    // before we instruct gRPC to open it. Note that here is a chance for a race condition. Someone
    // else could create a socket in between us checking and gRPC creating/overwriting the unix
    // socket. But due to gRPC's limitation there is only so much we can do about it.
    OUTCOME_TRY(VerifySocketAvailability(socket_path));
  }

  auto producer_side_server = std::make_unique<ProducerSideServer>();
  ORBIT_LOG("Starting producer-side server at %s", server_address);

  if (!producer_side_server->BuildAndStart(server_address)) {
    return ErrorMessage{"Unable to start producer-side server."};
  }

  ORBIT_LOG("Producer-side server is running");

  if (is_unix_domain_socket_connection) {
    std::string_view unix_socket_path = server_address.substr(kUnixDomainSocketPrefix.length());

    // When OrbitService runs as root, also allow non-root producers
    // (e.g., the game) to communicate over the Unix domain socket.
    std::error_code error_code{};
    std::filesystem::permissions(std::filesystem::path{unix_socket_path},
                                 std::filesystem::perms::all,
                                 std::filesystem::perm_options::replace, error_code);
    if (error_code) {
      producer_side_server->ShutdownAndWait();
      return ErrorMessage{absl::StrFormat("Changing mode bits to 777 of \"%s\": %s",
                                          unix_socket_path, error_code.message())};
    }
  }

  return producer_side_server;
}

}  // namespace orbit_producer_side_service