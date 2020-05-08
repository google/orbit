// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SESSION_H_
#define ORBIT_SSH_SESSION_H_

#include <libssh2.h>

#include <filesystem>
#include <memory>
#include <string>

#include "ResultType.h"
#include "Socket.h"

namespace OrbitSsh {

class Session {
 public:
  Session() = delete;
  Session(const Session&) = delete;
  Session& operator=(const Session&) = delete;
  Session(Session&&);
  Session& operator=(Session&&);
  ~Session();

  static std::optional<Session> Create();

  ResultType Handshake(Socket* socket_ptr);
  ResultType MatchKnownHosts(std::string host, int port,
                             std::filesystem::path known_hosts_path);
  ResultType Authenticate(std::string username, std::filesystem::path key_path,
                          std::string pass_phrase = "");
  ResultType Disconnect(std::string message = "Disconnecting normally");
  LIBSSH2_SESSION* GetRawSessionPtr() { return raw_session_ptr_; }
  void SetBlocking(bool value);

 private:
  explicit Session(LIBSSH2_SESSION* raw_session_ptr);
  LIBSSH2_SESSION* raw_session_ptr_ = nullptr;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SESSION_H_