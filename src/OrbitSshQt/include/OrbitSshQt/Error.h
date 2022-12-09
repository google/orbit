// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_ERROR_H_
#define ORBIT_SSH_QT_ERROR_H_

#include <string>
#include <system_error>
#include <type_traits>

namespace orbit_ssh_qt {

enum class Error {
  kNotConnected,
  kUncleanSessionShutdown,
  kUncleanChannelShutdown,
  kCouldNotListen,
  kRemoteSocketClosed,
  kLocalSocketClosed,
  kCouldNotOpenFile,
  kOrbitServiceShutdownTimedout
};

struct ErrorCategory : std::error_category {
  using std::error_category::error_category;

  [[nodiscard]] const char* name() const noexcept override { return "OrbitSshQt_Error"; }
  [[nodiscard]] std::string message(int condition) const override;
};

inline const ErrorCategory& GetErrorCategory() {
  static ErrorCategory category{};
  return category;
}

inline std::error_code make_error_code(Error e) {
  return std::error_code{static_cast<int>(e), GetErrorCategory()};
}

}  // namespace orbit_ssh_qt

namespace std {
template <>
struct is_error_condition_enum<orbit_ssh_qt::Error> : std::true_type {};
}  // namespace std
#endif  // ORBIT_SSH_QT_ERROR_H_