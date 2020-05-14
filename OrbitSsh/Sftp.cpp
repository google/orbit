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
    return static_cast<SftpError>(
        libssh2_session_last_errno(session->GetRawSessionPtr()));
  }
}

Sftp::Sftp(Sftp&& other) noexcept
    : raw_sftp_ptr_(other.raw_sftp_ptr_), session_(other.session_) {
  other.raw_sftp_ptr_ = nullptr;
  other.session_ = nullptr;
}

Sftp& Sftp::operator=(Sftp&& other) noexcept {
  raw_sftp_ptr_ = other.raw_sftp_ptr_;
  other.raw_sftp_ptr_ = nullptr;

  session_ = other.session_;
  other.session_ = nullptr;

  return *this;
}

outcome::result<void> Sftp::Shutdown() {
  CHECK(raw_sftp_ptr_);
  const auto result = libssh2_sftp_shutdown(raw_sftp_ptr_);

  if (result == 0) {
    raw_sftp_ptr_ = nullptr;
    return outcome::success();
  } else {
    return outcome::failure(static_cast<SftpError>(result));
  }
}

Sftp::~Sftp() {
  if (raw_sftp_ptr_) {
    (void)Shutdown();
  }
}
}  // namespace OrbitSsh
