// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_CHANNEL_H_
#define ORBIT_SSH_CHANNEL_H_

#include <libssh2.h>

#include <memory>
#include <optional>
#include <outcome.hpp>
#include <string>

#include "Error.h"
#include "Session.h"

namespace OrbitSsh {

class Channel {
 public:
  // This creates a tcp/ip channel to a third party via the remote ssh
  // server. In most cases this third party is a program running on the remote
  // server and therefore third_party_host is 127.0.0.1
  static outcome::result<Channel> OpenTcpIpTunnel(
      Session* session_ptr, const std::string& third_party_host,
      int third_party_port);
  static outcome::result<Channel> OpenChannel(Session* session_ptr);

  outcome::result<std::string> Read(int buffer_size = 0x400);
  outcome::result<void> WriteBlocking(std::string_view text);
  outcome::result<void> Exec(const std::string& command);
  outcome::result<void> RequestPty(const std::string& term);
  outcome::result<void> SendEOF();
  outcome::result<void> WaitRemoteEOF();
  outcome::result<void> Close();
  outcome::result<void> WaitClosed();

  int GetExitStatus();
  bool GetRemoteEOF();

 private:
  explicit Channel(LIBSSH2_CHANNEL* raw_channel_ptr);
  outcome::result<int> Write(std::string_view text);
  std::unique_ptr<LIBSSH2_CHANNEL, decltype(&libssh2_channel_free)>
      raw_channel_ptr_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_CHANNEL_H_
