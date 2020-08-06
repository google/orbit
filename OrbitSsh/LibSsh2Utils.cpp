// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "LibSsh2Utils.h"

namespace OrbitSsh {

std::string LibSsh2SessionLastError(LIBSSH2_SESSION* session) {
  // If the buffer is not thread-local this is unsafe - but if it isn't
  // there is no other way to do it really.
  char* error_msg = nullptr;
  int error_msg_len = 0;
  libssh2_session_last_error(session, &error_msg, &error_msg_len, false);

  if (error_msg == nullptr) {
    return {};
  }

  return std::string(error_msg, error_msg_len);
}

}  // namespace OrbitSsh