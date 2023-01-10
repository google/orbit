// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Credentials.h"

#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include <array>
#include <cstdlib>
#include <filesystem>
#include <string_view>
#include <utility>

#include "HomeEnvironmentVariable.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"

namespace orbit_ssh {

ErrorMessageOr<std::filesystem::path> GetUsersSshConfigDirectory() {
  const char* const home_directory_env_var = std::getenv(kHomeEnvironmentVariable);
  if (home_directory_env_var == nullptr) {
    return ErrorMessage{absl::StrFormat(
        "Could not determine the user's home directory location. Environment variable '%s' "
        "doesn't exist.",
        kHomeEnvironmentVariable)};
  }

  std::filesystem::path home_directory{home_directory_env_var};
  OUTCOME_TRY(bool home_dir_exists, orbit_base::IsDirectory(home_directory));

  if (!home_dir_exists) {
    return ErrorMessage{absl::StrFormat(
        "The user's home directory given by the '%s' environment variable does not exist "
        "or is not a directory. Tried the following directory: %s",
        kHomeEnvironmentVariable, home_directory.string())};
  }

  std::filesystem::path ssh_config_directory = home_directory / ".ssh";
  OUTCOME_TRY(bool ssh_config_dir_exists, orbit_base::IsDirectory(ssh_config_directory));
  if (!ssh_config_dir_exists) {
    return ErrorMessage{absl::StrFormat(
        "Could not find the user's SSH config directory. Tried the following directory: %s",
        ssh_config_directory.string())};
  }

  return ssh_config_directory;
}

ErrorMessageOr<Credentials> FindUsersCredentials(const std::filesystem::path& ssh_config_directory,
                                                 AddrAndPort addr_and_port, std::string username) {
  constexpr std::array kValidSshKeyFilenames{
      std::string_view{"id_ed25519"}, std::string_view{"id_ecdsa"}, std::string_view{"id_dsa"},
      std::string_view{"id_rsa"}};

  std::vector<std::filesystem::path> key_files;

  for (const auto& filename : kValidSshKeyFilenames) {
    std::filesystem::path path = ssh_config_directory / filename;

    auto result = orbit_base::IsRegularFile(path);
    if (result.has_error() || !result.value()) continue;

    key_files.emplace_back(path);
  }

  if (key_files.empty()) {
    return ErrorMessage{
        absl::StrFormat("No valid and/or supported SSH key files found! Checked files: %s/{%s}",
                        ssh_config_directory.string(), absl::StrJoin(kValidSshKeyFilenames, "|"))};
  }

  std::filesystem::path known_hosts_path = ssh_config_directory / "known_hosts";
  auto known_hosts_result = orbit_base::IsRegularFile(known_hosts_path);
  if (known_hosts_result.has_error() || !known_hosts_result.value()) {
    return ErrorMessage{
        absl::StrFormat("No known_hosts file found. Tried %s", known_hosts_path.string())};
  }

  return Credentials{.addr_and_port = std::move(addr_and_port),
                     .user = std::move(username),
                     .known_hosts_path = std::move(known_hosts_path),
                     .key_paths = std::move(key_files)};
}

}  // namespace orbit_ssh