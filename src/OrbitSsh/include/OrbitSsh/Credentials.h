// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SSH_CREDENTIALS_H_
#define ORBIT_SSH_SSH_CREDENTIALS_H_

#include <filesystem>
#include <string>

#include "OrbitSsh/AddrAndPort.h"

namespace orbit_ssh {

struct Credentials {
  AddrAndPort addr_and_port;
  std::string user;
  std::filesystem::path known_hosts_path;
  std::vector<std::filesystem::path> key_paths;
};

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_SSH_CREDENTIALS_H_