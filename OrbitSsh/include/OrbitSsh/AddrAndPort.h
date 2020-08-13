// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_ADDR_AND_PORT_H_
#define ORBIT_SSH_ADDR_AND_PORT_H_

#include <regex>
#include <string>
#include <tuple>

namespace OrbitSsh {

struct AddrAndPort {
  std::string addr;
  int port = -1;

  friend bool operator==(const AddrAndPort& lhs, const AddrAndPort& rhs) {
    return std::tie(lhs.addr, lhs.port) == std::tie(rhs.addr, rhs.port);
  }

  friend bool operator!=(const AddrAndPort& lhs, const AddrAndPort& rhs) {
    return !(lhs == rhs);
  }
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_ADDR_AND_PORT_H_