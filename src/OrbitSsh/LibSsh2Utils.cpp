// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibSsh2Utils.h"

#include <absl/strings/str_format.h>

namespace orbit_ssh {

std::pair<int, std::string> LibSsh2SessionLastError(LIBSSH2_SESSION* session) {
  // If the buffer is not thread-local this is unsafe - but if it isn't
  // there is no other way to do it really.
  char* error_msg = nullptr;
  int error_msg_len = 0;
  int last_errno = libssh2_session_last_error(session, &error_msg, &error_msg_len, 0);

  if (error_msg == nullptr) {
    return {};
  }

  return std::make_pair(
      last_errno,
      absl::StrFormat("%s (errno: %d)", std::string(error_msg, error_msg_len), last_errno));
}

}  // namespace orbit_ssh