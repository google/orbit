// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SFTP_H
#define ORBIT_SSH_SFTP_H

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <outcome.hpp>

#include "OrbitSsh/Error.h"
#include "OrbitSsh/Session.h"

namespace OrbitSsh {

class Sftp {
 public:
  static outcome::result<Sftp> Init(Session* session);

  Sftp(const Sftp&) = delete;
  Sftp& operator=(const Sftp&) = delete;

  Sftp(Sftp&&) noexcept;
  Sftp& operator=(Sftp&&) noexcept;

  ~Sftp();
  outcome::result<void> Shutdown();

  LIBSSH2_SFTP* GetRawSftpPtr() const noexcept { return raw_sftp_ptr_; }
  Session* GetSession() const noexcept { return session_; }

 private:
  explicit Sftp(LIBSSH2_SFTP* raw_sftp_ptr, Session* session)
      : raw_sftp_ptr_(raw_sftp_ptr), session_(session) {}
  LIBSSH2_SFTP* raw_sftp_ptr_ = nullptr;
  Session* session_ = nullptr;
};

}  // namespace OrbitSsh
#endif  // ORBIT_SSH_SFTP_H
