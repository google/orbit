// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_CHANNEL_H_
#define ORBIT_SSH_CHANNEL_H_

#include <libssh2.h>
#include <stddef.h>

#include <memory>
#include <optional>
#include <string>
#include <string_view>

#include "OrbitBase/Result.h"
#include "Session.h"

namespace orbit_ssh {

class Channel {
 public:
  // This creates a tcp/ip channel to a third party via the remote ssh
  // server. In most cases this third party is a program running on the remote
  // server and therefore third_party_host is 127.0.0.1
  static outcome::result<Channel> OpenTcpIpTunnel(Session* session_ptr,
                                                  std::string_view third_party_host,
                                                  int third_party_port);
  static outcome::result<Channel> OpenChannel(Session* session_ptr);

  outcome::result<std::string> ReadStdOut(int buffer_size = 0x400);
  outcome::result<std::string> ReadStdErr(int buffer_size = 0x400);

  // Returns the accumulated number of bytes available to read from all data streams. That's usually
  // stdout and stderr but there could be more streams.
  size_t GetNumBytesToRead();

  outcome::result<void> WriteBlocking(std::string_view text);
  outcome::result<int> Write(std::string_view text);
  outcome::result<void> Exec(std::string_view command);
  outcome::result<void> SendEOF();
  outcome::result<void> WaitRemoteEOF();
  outcome::result<void> Close();
  outcome::result<void> WaitClosed();

  int GetExitStatus();
  bool GetRemoteEOF();

 private:
  explicit Channel(LIBSSH2_CHANNEL* raw_channel_ptr);
  std::unique_ptr<LIBSSH2_CHANNEL, decltype(&libssh2_channel_free)> raw_channel_ptr_;
};

}  // namespace orbit_ssh

#endif  // ORBIT_SSH_CHANNEL_H_
