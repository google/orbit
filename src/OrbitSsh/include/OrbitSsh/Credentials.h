// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SSH_CREDENTIALS_H_
#define ORBIT_SSH_SSH_CREDENTIALS_H_

#include <filesystem>
#include <string>
#include <vector>

#include "OrbitBase/Result.h"
#include "OrbitSsh/AddrAndPort.h"

namespace orbit_ssh {

struct Credentials {
  AddrAndPort addr_and_port;
  std::string user;
  std::filesystem::path known_hosts_path;
  std::vector<std::filesystem::path> key_paths;
};

// Returns the path to the `~/.ssh` directory inside the user's home directory. Returns an error if
// the directory doesn't exist, but if it does no further checks are performed. Use
// `FindUsersCredentials` below to check whether the `.ssh` directory contains the necessary files.
ErrorMessageOr<std::filesystem::path> GetUsersSshConfigDirectory();

// Constructs an instance of `Credentials` with the given address, port and username, and all the
// supported SSH keys found in `ssh_config_directory`. Returns an error if no supported key was
// found. It also requires a `known_hosts` file to exist since we currently don't support writing to
// or creating known_hosts files. This might change in the future.
ErrorMessageOr<Credentials> FindUsersCredentials(const std::filesystem::path& ssh_config_directory,
                                                 AddrAndPort addr_and_port, std::string username);

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_SSH_CREDENTIALS_H_