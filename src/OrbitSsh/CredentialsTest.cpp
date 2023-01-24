// Copyright (c) 2023 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdlib>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include "HomeEnvironmentVariable.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Credentials.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh {
using orbit_test_utils::HasErrorWithMessage;
using orbit_test_utils::HasValue;
using testing::Field;
using testing::UnorderedElementsAre;

TEST(GetUsersSshConfigDirectory, FindsSshDirectory) {
  std::string original_home_env_var_value = [&]() {
    const char* env = std::getenv(kHomeEnvironmentVariable);
    EXPECT_THAT(env, testing::Ne(nullptr));
    return std::string{env != nullptr ? env : ""};
  }();

#ifdef _WIN32
  EXPECT_THAT(_putenv_s(kHomeEnvironmentVariable, orbit_test::GetTestdataDir().string().c_str()),
              0);
#else
  EXPECT_THAT(setenv(kHomeEnvironmentVariable, orbit_test::GetTestdataDir().string().c_str(), 1),
              0);
#endif

  EXPECT_THAT(GetUsersSshConfigDirectory(),
              orbit_test_utils::HasValue(orbit_test::GetTestdataDir() / ".ssh"));

  // Reset HOME environment variable
#ifdef _WIN32
  EXPECT_THAT(_putenv_s(kHomeEnvironmentVariable, original_home_env_var_value.c_str()), 0);
#else
  EXPECT_THAT(setenv(kHomeEnvironmentVariable, original_home_env_var_value.c_str(), 1), 0);
#endif
}

TEST(FindUsersCredentials, AcceptsValidSshConfigDir) {
  AddrAndPort addr_and_port{"host", 1024};
  const std::string username{"nodody"};

  const std::filesystem::path ssh_config_dir = orbit_test::GetTestdataDir() / "valid_ssh_config";
  auto result = FindUsersCredentials(ssh_config_dir, addr_and_port, username);

  EXPECT_THAT(result, HasValue(Field(&Credentials::addr_and_port, addr_and_port)));
  EXPECT_THAT(result, HasValue(Field(&Credentials::user, username)));
  EXPECT_THAT(result,
              HasValue(Field(&Credentials::known_hosts_path, ssh_config_dir / "known_hosts")));
  EXPECT_THAT(result, HasValue(Field(&Credentials::key_paths,
                                     UnorderedElementsAre(ssh_config_dir / "id_ecdsa",
                                                          ssh_config_dir / "id_rsa"))));
}

TEST(FindUsersCredentials, FailsWhenKnownHostsMissing) {
  AddrAndPort addr_and_port{"host", 1024};
  const std::string username{"nodody"};

  const std::filesystem::path ssh_config_dir =
      orbit_test::GetTestdataDir() / "missing_known_hosts_ssh_config";
  auto result = FindUsersCredentials(ssh_config_dir, addr_and_port, username);

  EXPECT_THAT(result, HasErrorWithMessage("No known_hosts file found"));
}

TEST(FindUsersCredentials, FailsWhenNoKeyFound) {
  AddrAndPort addr_and_port{"host", 1024};
  const std::string username{"nodody"};

  const std::filesystem::path ssh_config_dir =
      orbit_test::GetTestdataDir() / "missing_key_ssh_config";
  auto result = FindUsersCredentials(ssh_config_dir, addr_and_port, username);

  EXPECT_THAT(result, HasErrorWithMessage("No valid and/or supported SSH key files found"));
}

}  // namespace orbit_ssh