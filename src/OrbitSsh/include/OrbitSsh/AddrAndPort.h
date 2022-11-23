// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_ADDR_AND_PORT_H_
#define ORBIT_SSH_ADDR_AND_PORT_H_

#include <absl/strings/str_format.h>

#include <regex>
#include <string>
#include <tuple>

namespace orbit_ssh {

struct AddrAndPort {
  std::string addr;
  int port = -1;

  friend bool operator==(const AddrAndPort& lhs, const AddrAndPort& rhs) {
    return std::tie(lhs.addr, lhs.port) == std::tie(rhs.addr, rhs.port);
  }

  friend bool operator!=(const AddrAndPort& lhs, const AddrAndPort& rhs) { return !(lhs == rhs); }

  [[nodiscard]] std::string GetHumanReadable() const {
    return absl::StrFormat("%s:%d", addr, port);
  }
};

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_ADDR_AND_PORT_H_