// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Channel.h"

#include <libssh2.h>

#include <memory>
#include <optional>
#include <outcome.hpp>

#include "OrbitBase/Logging.h"

namespace OrbitSsh {

Channel::Channel(LIBSSH2_CHANNEL* raw_channel_ptr)
    : raw_channel_ptr_(raw_channel_ptr, &libssh2_channel_free) {}

outcome::result<Channel> Channel::OpenChannel(Session* session) {
  LIBSSH2_CHANNEL* raw_channel_ptr = libssh2_channel_open_session(session->GetRawSessionPtr());
  if (raw_channel_ptr != nullptr) {
    return outcome::success(Channel{raw_channel_ptr});
  }

  return static_cast<Error>(libssh2_session_last_errno(session->GetRawSessionPtr()));
}

outcome::result<Channel> Channel::OpenTcpIpTunnel(Session* session,
                                                  const std::string& third_party_host,
                                                  int third_party_port) {
  LIBSSH2_CHANNEL* raw_channel_ptr = libssh2_channel_direct_tcpip(
      session->GetRawSessionPtr(), third_party_host.c_str(), third_party_port);
  if (raw_channel_ptr != nullptr) {
    return outcome::success(Channel{raw_channel_ptr});
  }

  return static_cast<Error>(libssh2_session_last_errno(session->GetRawSessionPtr()));
}

outcome::result<void> Channel::Exec(const std::string& command) {
  const int rc = libssh2_channel_exec(raw_channel_ptr_.get(), command.c_str());
  if (rc == 0) {
    return outcome::success();
  }

  return static_cast<Error>(rc);
}

outcome::result<std::string> Channel::ReadStdOut(int buffer_size) {
  std::string buffer(buffer_size, '\0');
  const int rc = libssh2_channel_read(raw_channel_ptr_.get(), buffer.data(), buffer.size());

  if (rc < 0) return static_cast<Error>(rc);

  buffer.resize(rc);
  return outcome::success(std::move(buffer));
}

outcome::result<std::string> Channel::ReadStdErr(int buffer_size) {
  std::string buffer(buffer_size, '\0');
  const int rc = libssh2_channel_read_stderr(raw_channel_ptr_.get(), buffer.data(), buffer.size());

  if (rc < 0) return static_cast<Error>(rc);

  buffer.resize(rc);
  return outcome::success(std::move(buffer));
}

outcome::result<int> Channel::Write(std::string_view text) {
  const int rc = libssh2_channel_write(raw_channel_ptr_.get(), text.data(), text.size());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success(rc);
}

outcome::result<void> Channel::WriteBlocking(std::string_view text) {
  do {
    const auto result = Write(text);
    if (!result && result.error() != make_error_code(Error::kEagain)) {
      return result.error();
    }
    text = text.substr(result.value());
  } while (!text.empty());

  return outcome::success();
}

outcome::result<void> Channel::SendEOF() {
  const int rc = libssh2_channel_send_eof(raw_channel_ptr_.get());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

outcome::result<void> Channel::WaitRemoteEOF() {
  const int rc = libssh2_channel_wait_eof(raw_channel_ptr_.get());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

outcome::result<void> Channel::Close() {
  const int rc = libssh2_channel_close(raw_channel_ptr_.get());

  if (rc < 0) return static_cast<Error>(rc);

  raw_channel_ptr_.reset();
  return outcome::success();
}

outcome::result<void> Channel::WaitClosed() {
  const int rc = libssh2_channel_wait_closed(raw_channel_ptr_.get());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

int Channel::GetExitStatus() { return libssh2_channel_get_exit_status(raw_channel_ptr_.get()); }

bool Channel::GetRemoteEOF() { return libssh2_channel_eof(raw_channel_ptr_.get()) == 1; }

}  // namespace OrbitSsh
