// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <sys/stat.h>

#include <filesystem>
#include <system_error>

#include "BuildAndStartProducerSideServerWithUri.h"
#include "OrbitBase/SafeStrerror.h"
#include "ProducerSideChannel/ProducerSideChannel.h"
#include "ProducerSideService/BuildAndStartProducerSideServer.h"

namespace orbit_producer_side_service {

std::unique_ptr<ProducerSideServer> BuildAndStartProducerSideServer() {
  const std::filesystem::path unix_domain_socket_dir =
      std::filesystem::path{orbit_producer_side_channel::kProducerSideUnixDomainSocketPath}
          .parent_path();
  std::error_code error_code;
  std::filesystem::create_directories(unix_domain_socket_dir, error_code);
  if (error_code) {
    ORBIT_ERROR("Unable to create directory for socket for producer-side server: %s",
                error_code.message());
    return nullptr;
  }

  std::string unix_socket_path(orbit_producer_side_channel::kProducerSideUnixDomainSocketPath);
  std::string uri = absl::StrFormat("unix:%s", unix_socket_path);
  auto producer_side_server = BuildAndStartProducerSideServerWithUri(uri);

  // When OrbitService runs as root, also allow non-root producers
  // (e.g., the game) to communicate over the Unix domain socket.
  if (chmod(unix_socket_path.c_str(), 0777) != 0) {
    ORBIT_ERROR("Changing mode bits to 777 of \"%s\": %s", unix_socket_path, SafeStrerror(errno));
    producer_side_server->ShutdownAndWait();
    return nullptr;
  }

  return producer_side_server;
}

}  // namespace orbit_producer_side_service