// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_SSH_SESSION_TEST_H_
#define ORBIT_SSH_QT_SSH_SESSION_TEST_H_

#include <absl/strings/numbers.h>
#include <absl/strings/str_replace.h>
#include <absl/strings/str_split.h>
#include <gtest/gtest.h>

#include <optional>
#include <string_view>
#include <system_error>

#include "OrbitBase/File.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/WriteStringToFile.h"
#include "OrbitSsh/Credentials.h"
#include "Test/Path.h"
#include "TestUtils/TemporaryDirectory.h"
#include "TestUtils/TestUtils.h"

namespace orbit_ssh_qt {

// A test fixture for SSH tests that does the following:
// - Skips tests if no SSH server has been advertised
// - Offers the GetCredentials() member function which returns all the information needed to make an
// SSH connection to the test server.
class SshSessionTest : public testing::Test {
 public:
  // We can't use the constructor here because neither GTEST_SKIP, nor ASSERT_THAT are supported in
  // constructors.
  void SetUp() override {
    char* orbit_testing_ssh_server = std::getenv("ORBIT_TESTING_SSH_SERVER_SIMPLE_ADDRESS");
    if (orbit_testing_ssh_server == nullptr) {
      GTEST_SKIP() << "No SSH server provided. Skipping test.";
    }
    RecordProperty("server_address", orbit_testing_ssh_server);

    ErrorMessageOr<orbit_test_utils::TemporaryDirectory> temp_dir =
        orbit_test_utils::TemporaryDirectory::Create();
    ASSERT_THAT(temp_dir, orbit_test_utils::HasNoError());
    temp_dir_ = std::move(temp_dir).value();

    ErrorMessageOr<orbit_ssh::Credentials> credentials =
        CreateCredentials(orbit_testing_ssh_server);
    ASSERT_THAT(credentials, orbit_test_utils::HasNoError());
    credentials_ = std::move(credentials).value();
  }

  [[nodiscard]] orbit_ssh::Credentials GetCredentials() const { return credentials_; }

 private:
  ErrorMessageOr<std::string> CreateKnownHostsFile(std::string_view host, int port) const {
    OUTCOME_TRY(std::string contents,
                orbit_base::ReadFileToString(orbit_test::GetTestdataDir() / "known_hosts.in"));
    absl::StrReplaceAll({{"%HOSTNAME%", host}, {"%PORT%", std::to_string(port)}}, &contents);

    std::string path = (temp_dir_->GetDirectoryPath() / "known_hosts").string();
    OUTCOME_TRY(orbit_base::WriteStringToFile(path, contents));

    return path;
  }

  ErrorMessageOr<orbit_ssh::Credentials> CreateCredentials(
      std::string_view ssh_server_address) const {
    orbit_ssh::Credentials credentials{};
    credentials.user = "loginuser";

    std::vector<std::string_view> split_server_address = absl::StrSplit(ssh_server_address, ':');

    if (split_server_address.size() != 2) {
      return ErrorMessage{"Expected hostname and port divided by :."};
    }

    int port{};
    if (!absl::SimpleAtoi(split_server_address.back(), &port)) {
      return ErrorMessage{"Couldn't parse port number"};
    }

    credentials.addr_and_port.addr = split_server_address.front();
    credentials.addr_and_port.port = port;

    OUTCOME_TRY(credentials.known_hosts_path, CreateKnownHostsFile(credentials.addr_and_port.addr,
                                                                   credentials.addr_and_port.port));

    credentials.key_path = orbit_test::GetTestdataDir() / "id_ed25519";
    EXPECT_THAT(orbit_base::FileOrDirectoryExists(credentials.key_path),
                orbit_test_utils::HasValue(true));

    return credentials;
  }

  std::optional<orbit_test_utils::TemporaryDirectory> temp_dir_;
  orbit_ssh::Credentials credentials_;
};
}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_SSH_SESSION_TEST_H_