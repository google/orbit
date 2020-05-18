// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Sftp.h"

#include "OrbitBase/Logging.h"

namespace OrbitSsh {

outcome::result<Sftp> Sftp::Init(Session* session) {
  CHECK(session);
  const auto result = libssh2_sftp_init(session->GetRawSessionPtr());

  if (result) {
    return Sftp{result, session};
  } else {
    return static_cast<Error>(
        libssh2_session_last_errno(session->GetRawSessionPtr()));
  }
}

outcome::result<void> Sftp::Shutdown() {
  CHECK(raw_sftp_ptr_);
  const auto result = libssh2_sftp_shutdown(raw_sftp_ptr_.get());

  if (result == 0) {
    raw_sftp_ptr_.release();
    return outcome::success();
  } else {
    return outcome::failure(static_cast<Error>(result));
  }
}

}  // namespace OrbitSsh
