// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SSH_CREDENTIALS_H_
#define ORBIT_SSH_SSH_CREDENTIALS_H_

#include <filesystem>
#include <string>

namespace OrbitSsh {

struct Credentials {
  std::string host;
  int port = -1;
  std::string user;
  std::filesystem::path known_hosts_path;
  std::filesystem::path key_path;

  bool Empty() const {
    return host.empty() && port == -1 && user.empty() &&
           known_hosts_path.empty() && key_path.empty();
  }
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SSH_CREDENTIALS_H_