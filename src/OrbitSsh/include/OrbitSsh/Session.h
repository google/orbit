// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SESSION_H_
#define ORBIT_SSH_SESSION_H_

#include <libssh2.h>

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

#include "OrbitBase/Result.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Socket.h"

namespace orbit_ssh {

class Session {
 public:
  static outcome::result<Session> Create(const Context* context);

  outcome::result<void> Handshake(Socket* socket_ptr);
  outcome::result<void> MatchKnownHosts(const AddrAndPort& addr_and_port,
                                        const std::filesystem::path& known_hosts_path);
  outcome::result<void> Authenticate(std::string_view username,
                                     const std::filesystem::path& key_path,
                                     std::string_view pass_phrase = "");
  outcome::result<void> Disconnect(std::string_view message = "Disconnecting normally");
  [[nodiscard]] LIBSSH2_SESSION* GetRawSessionPtr() const { return raw_session_ptr_.get(); }
  void SetBlocking(bool value);

 private:
  explicit Session(LIBSSH2_SESSION* raw_session_ptr);
  std::unique_ptr<LIBSSH2_SESSION, decltype(&libssh2_session_free)> raw_session_ptr_;
};

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_SESSION_H_
