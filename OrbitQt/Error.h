// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ERROR_H_
#define ORBIT_QT_ERROR_H_

#include <system_error>

namespace OrbitQt {

enum class Error {
  kCouldNotConnectToServer,
  kCouldNotUploadPackage,
  kCouldNotUploadSignature,
  kCouldNotInstallPackage,
  kCouldNotStartTunnel
};

struct ErrorCategory : std::error_category {
  using std::error_category::error_category;

  const char* name() const noexcept override { return "OrbitQt_Error"; }
  std::string message(int condition) const override;
};

inline const ErrorCategory& GetErrorCategory() {
  static ErrorCategory category{};
  return category;
}

inline std::error_code make_error_code(Error e) {
  return std::error_code{static_cast<int>(e), GetErrorCategory()};
}

}  // namespace OrbitQt

namespace std {
template <>
struct is_error_condition_enum<OrbitQt::Error> : std::true_type {};
}  // namespace std

#endif  // ORBIT_QT_ERROR_H_