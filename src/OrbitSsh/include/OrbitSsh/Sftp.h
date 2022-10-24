// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SFTP_H
#define ORBIT_SSH_SFTP_H

#include <libssh2.h>
#include <libssh2_sftp.h>

#include <memory>

#include "OrbitBase/Result.h"
#include "OrbitSsh/Session.h"

namespace orbit_ssh {

class Sftp {
 public:
  static outcome::result<Sftp> Init(Session* session);

  outcome::result<void> Shutdown();

  [[nodiscard]] LIBSSH2_SFTP* GetRawSftpPtr() const { return raw_sftp_ptr_.get(); }
  [[nodiscard]] Session* GetSession() const { return session_; }

 private:
  explicit Sftp(LIBSSH2_SFTP* raw_sftp_ptr, Session* session)
      : raw_sftp_ptr_(raw_sftp_ptr, &libssh2_sftp_shutdown), session_(session) {}

  std::unique_ptr<LIBSSH2_SFTP, decltype(&libssh2_sftp_shutdown)> raw_sftp_ptr_;
  Session* session_ = nullptr;
};

}  // namespace orbit_ssh
#endif  // ORBIT_SSH_SFTP_H
