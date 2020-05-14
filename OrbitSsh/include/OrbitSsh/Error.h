// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_ERROR_H_
#define ORBIT_SSH_ERROR_H_

#include <libssh2_sftp.h>

#include <system_error>

namespace OrbitSsh {

enum class SftpError {
  kAlloc = LIBSSH2_ERROR_ALLOC,
  kSocketSend = LIBSSH2_ERROR_SOCKET_SEND,
  kSocketTimeout = LIBSSH2_ERROR_SOCKET_TIMEOUT,
  kSftpProtocol = LIBSSH2_ERROR_SFTP_PROTOCOL,
  kEagain = LIBSSH2_ERROR_EAGAIN
};

struct SftpErrorCategory : std::error_category {
  using std::error_category::error_category;

  const char* name() const noexcept override { return "libssh2_sftp"; }
  std::string message(int condition) const override;
  std::error_condition default_error_condition(int c) const noexcept override {
    if (static_cast<SftpError>(c) == SftpError::kEagain) {
      return std::make_error_condition(
          std::errc::resource_unavailable_try_again);
    } else {
      return std::error_condition{c, *this};
    }
  }
};

inline const SftpErrorCategory& GetSftpErrorCategory() {
  static SftpErrorCategory category{};
  return category;
}

inline std::error_code make_error_code(SftpError e) {
  return std::error_code{static_cast<int>(e), GetSftpErrorCategory()};
}

}  // namespace OrbitSsh

namespace std {
template <>
struct is_error_condition_enum<OrbitSsh::SftpError> : std::true_type {};
}  // namespace std
#endif  // ORBIT_SSH_ERROR_H_
