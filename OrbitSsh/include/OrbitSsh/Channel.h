// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_CHANNEL_H_
#define ORBIT_SSH_CHANNEL_H_

#include <libssh2.h>

#include <memory>
#include <optional>
#include <string>

#include "ResultType.h"
#include "Session.h"

namespace OrbitSsh {

class Channel {
 public:
  Channel() = delete;
  Channel(const Channel&) = delete;
  Channel& operator=(const Channel&) = delete;
  Channel(Channel&& other);
  Channel& operator=(Channel&& other);
  ~Channel();

  // This creates a tcp/ip channel to a third party via the remote ssh
  // server. In most cases this third party is a program running on the remote
  // server and therefore third_party_host is 127.0.0.1
  static ResultType CreateDirectTcpIp(Session* session_ptr,
                                      const std::string& third_party_host,
                                      int third_party_port,
                                      std::optional<Channel>* channel_opt);
  static ResultType CreateOpenSession(Session* session_ptr,
                                      std::optional<Channel>* channel_opt);

  ResultType Read(std::string* result, int buffer_size = 0x400);
  ResultType WriteBlocking(const std::string& text);
  ResultType Exec(const std::string& command);
  ResultType RequestPty(const std::string& term);
  ResultType SendEOF();
  ResultType WaitRemoteEOF();
  ResultType Close();
  ResultType WaitClosed();

  int GetExitStatus();
  bool GetRemoteEOF();

 private:
  explicit Channel(LIBSSH2_CHANNEL* raw_channel_ptr);
  ResultType Write(const std::string& text, size_t* written_length);
  LIBSSH2_CHANNEL* raw_channel_ptr_;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_CHANNEL_H_