// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SESSION_H_
#define ORBIT_SSH_SESSION_H_

#include <libssh2.h>

#include <filesystem>
#include <memory>
#include <outcome.hpp>
#include <string>

#include "Socket.h"

namespace OrbitSsh {

class Session {
 public:
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;

  Session(Session&&) noexcept;
  Session& operator=(Session&&) noexcept;
  ~Session() noexcept;

  static outcome::result<Session> Create();

  outcome::result<void> Handshake(Socket* socket_ptr);
  outcome::result<void> MatchKnownHosts(std::string host, int port,
                                        std::filesystem::path known_hosts_path);
  outcome::result<void> Authenticate(std::string username,
                                     std::filesystem::path key_path,
                                     std::string pass_phrase = "");
  outcome::result<void> Disconnect(
      std::string message = "Disconnecting normally");
  LIBSSH2_SESSION* GetRawSessionPtr() const noexcept {
    return raw_session_ptr_;
  }
  void SetBlocking(bool value);

 private:
  explicit Session(LIBSSH2_SESSION* raw_session_ptr);
  LIBSSH2_SESSION* raw_session_ptr_ = nullptr;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SESSION_H_
