// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SshQtTestUtils/ParsePortNumberFromSocatOutput.h"

#include <absl/strings/match.h>
#include <absl/strings/numbers.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_split.h>

#include <string_view>

namespace orbit_ssh_qt_test_utils {

std::optional<ErrorMessageOr<int>> ParsePortNumberFromSocatOutput(std::string_view socat_output) {
  // The relevant information is in the first line of the stderr output. So we wait until we have
  // received the first line break.
  if (!absl::StrContains(socat_output, '\n')) {
    return std::nullopt;
  }

  auto lines = absl::StrSplit(socat_output, '\n');
  std::string_view first_line = *lines.begin();

  constexpr std::string_view kIpAddressAndColon{"0.0.0.0:"};
  auto ip_location = first_line.find(kIpAddressAndColon);
  if (ip_location == std::string_view::npos) {
    return ErrorMessage{
        absl::StrCat("Couldn't find the IP address in the first line: ", first_line)};
  }

  std::string_view port_as_string = first_line.substr(ip_location + kIpAddressAndColon.size());
  int port{};
  if (!absl::SimpleAtoi(port_as_string, &port)) {
    return ErrorMessage{absl::StrCat("Couldn't parse port number. Input was: ", port_as_string)};
  }

  return port;
}
}  // namespace orbit_ssh_qt_test_utils
