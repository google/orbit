// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Sftp.h"

#include <libssh2.h>

#include "LibSsh2Utils.h"
#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"

namespace orbit_ssh {

outcome::result<Sftp> Sftp::Init(Session* session) {
  ORBIT_CHECK(session);
  auto* const result = libssh2_sftp_init(session->GetRawSessionPtr());

  if (result == nullptr) {
    const auto [last_errno, error_message] = LibSsh2SessionLastError(session->GetRawSessionPtr());
    if (last_errno != LIBSSH2_ERROR_EAGAIN) {
      ORBIT_ERROR("Call to ligssh2_sftp_init() failed. Last session error message: %s",
                  error_message);
    }
    return static_cast<Error>(last_errno);
  }

  return Sftp{result, session};
}

outcome::result<void> Sftp::Shutdown() {
  ORBIT_CHECK(raw_sftp_ptr_);
  const auto result = libssh2_sftp_shutdown(raw_sftp_ptr_.get());

  if (result < 0) {
    return outcome::failure(static_cast<Error>(result));
  }

  (void)raw_sftp_ptr_.release();
  return outcome::success();
}

}  // namespace orbit_ssh
