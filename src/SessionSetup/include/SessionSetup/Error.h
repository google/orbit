// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_ERROR_H_
#define SESSION_SETUP_ERROR_H_

#include <QMetaType>
#include <string>
#include <system_error>
#include <type_traits>

namespace orbit_session_setup {

enum class Error {
  kCouldNotConnectToServer,
  kCouldNotUploadPackage,
  kCouldNotUploadSignature,
  kCouldNotInstallPackage,
  kCouldNotStartTunnel,
  kUserCanceledServiceDeployment
};

struct ErrorCategory : std::error_category {
  using std::error_category::error_category;

  [[nodiscard]] const char* name() const noexcept override { return "OrbitQt_Error"; }
  [[nodiscard]] std::string message(int condition) const override;
};

inline const ErrorCategory& GetErrorCategory() {
  static ErrorCategory category{};
  return category;
}

inline std::error_code make_error_code(Error e) {
  return std::error_code{static_cast<int>(e), GetErrorCategory()};
}

}  // namespace orbit_session_setup

namespace std {
template <>
struct is_error_condition_enum<orbit_session_setup::Error> : std::true_type {};
}  // namespace std

Q_DECLARE_METATYPE(std::error_code);

#endif  // SESSION_SETUP_ERROR_H_