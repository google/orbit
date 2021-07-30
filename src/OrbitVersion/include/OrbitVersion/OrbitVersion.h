// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_VERSION_ORBIT_VERSION_H_
#define ORBIT_VERSION_ORBIT_VERSION_H_

#include <string>

namespace orbit_version {

struct Version final {
  uint32_t major_version;
  uint32_t minor_version;

  friend bool operator<(const Version& left, const Version& right) {
    return std::tie(left.major_version, left.minor_version) <
           std::tie(right.major_version, right.minor_version);
  }

  friend bool operator<=(const Version& left, const Version& right) {
    return std::tie(left.major_version, left.minor_version) <=
           std::tie(right.major_version, right.minor_version);
  }

  friend bool operator==(const Version& left, const Version& right) {
    return std::tie(left.major_version, left.minor_version) ==
           std::tie(right.major_version, right.minor_version);
  }

  friend bool operator!=(const Version& left, const Version& right) {
    return std::tie(left.major_version, left.minor_version) !=
           std::tie(right.major_version, right.minor_version);
  }

  friend bool operator>=(const Version& left, const Version& right) { return right <= left; }

  friend bool operator>(const Version& left, const Version& right) { return right < left; }
};

[[nodiscard]] Version GetVersion();
std::string GetVersionString();
std::string GetCompiler();
std::string GetBuildTimestamp();
std::string GetBuildMachine();
std::string GetCommitHash();

// For usage with a "--version" command line flag
std::string GetBuildReport();

}  // namespace orbit_version

#endif  // ORBIT_VERSION_ORBIT_VERSION_H_
