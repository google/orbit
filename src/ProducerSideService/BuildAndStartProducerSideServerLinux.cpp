// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <system_error>

#include "BuildAndStartProducerSideServerWithUri.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "ProducerSideService/BuildAndStartProducerSideServer.h"
#include "ProducerSideService/ProducerSideServer.h"

namespace orbit_producer_side_service {

// Tries to connect to the unix socket associated with the given path. If the connection succeeds we
// know that something is already listening there.
static ErrorMessageOr<void> VerifySocketAvailability(std::string_view socket_path) {
  int socket_fd = socket(AF_UNIX, SOCK_STREAM, 0);

  sockaddr_un addr{};
  std::memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  socket_path.copy(addr.sun_path, sizeof(addr.sun_path) - 1);

  if (connect(socket_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0) {
    close(socket_fd);

    // TODO(b/229048915): Update the error message once the cloud collector is released.
    return ErrorMessage{"OrbitService is already running on the instance."};
  }

  return outcome::success();
}

ErrorMessageOr<std::unique_ptr<ProducerSideServer>> BuildAndStartProducerSideServer() {
  const std::filesystem::path unix_domain_socket_dir =
      std::filesystem::path{orbit_producer_side_channel::kProducerSideUnixDomainSocketPath}
          .parent_path();
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
  OUTCOME_TRY(
      VerifySocketAvailability(orbit_producer_side_channel::kProducerSideUnixDomainSocketPath));

  std::string unix_socket_path(orbit_producer_side_channel::kProducerSideUnixDomainSocketPath);
  std::string uri = absl::StrFormat("unix:%s", unix_socket_path);
  OUTCOME_TRY(std::unique_ptr<ProducerSideServer> producer_side_server,
              BuildAndStartProducerSideServerWithUri(uri));

  // When OrbitService runs as root, also allow non-root producers
  // (e.g., the game) to communicate over the Unix domain socket.
  if (chmod(unix_socket_path.c_str(), 0777) != 0) {
    producer_side_server->ShutdownAndWait();
    return ErrorMessage{absl::StrFormat("Changing mode bits to 777 of \"%s\": %s", unix_socket_path,
                                        SafeStrerror(errno))};
  }

  return producer_side_server;
}

}  // namespace orbit_producer_side_service