// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Channel.h"

#include <libssh2.h>

#include <memory>
#include <optional>
#include <outcome.hpp>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"

namespace OrbitSsh {

Channel::Channel(LIBSSH2_CHANNEL* raw_channel_ptr)
    : raw_channel_ptr_(raw_channel_ptr) {}

Channel::Channel(Channel&& other) {
  raw_channel_ptr_ = other.raw_channel_ptr_;
  other.raw_channel_ptr_ = nullptr;
}

Channel& Channel::operator=(Channel&& other) {
  if (this != &other) {
    raw_channel_ptr_ = other.raw_channel_ptr_;
    other.raw_channel_ptr_ = nullptr;
  }
  return *this;
}

Channel::~Channel() {
  if (raw_channel_ptr_ != nullptr) {
    libssh2_channel_free(raw_channel_ptr_);
  }
}

ResultType Channel::CreateOpenSession(Session* session_ptr,
                                      std::optional<Channel>* channel_opt) {
  LIBSSH2_CHANNEL* raw_channel_ptr =
      libssh2_channel_open_session(session_ptr->GetRawSessionPtr());
  if (raw_channel_ptr != nullptr) {
    *channel_opt = Channel(raw_channel_ptr);
    return ResultType::kSuccess;
  }

  int last_errno = libssh2_session_last_errno(session_ptr->GetRawSessionPtr());
  if (last_errno == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Unable to open channel, last errorno %d", last_errno);
  return ResultType::kError;
}

ResultType Channel::CreateDirectTcpIp(Session* session_ptr,
                                      const std::string& third_party_host,
                                      int third_party_port,
                                      std::optional<Channel>* channel_opt) {
  LIBSSH2_CHANNEL* raw_channel_ptr =
      libssh2_channel_direct_tcpip(session_ptr->GetRawSessionPtr(),
                                   third_party_host.c_str(), third_party_port);
  if (raw_channel_ptr != nullptr) {
    *channel_opt = Channel(raw_channel_ptr);
    return ResultType::kSuccess;
  }

  int last_errno = libssh2_session_last_errno(session_ptr->GetRawSessionPtr());
  if (last_errno == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  // TODO (antonrohr) better error handling. (resolve that in higher level)
  if (last_errno == LIBSSH2_ERROR_CHANNEL_FAILURE) return ResultType::kAgain;

  ERROR("Unable to open channel, last errno %d", last_errno);
  return ResultType::kError;
}

ResultType Channel::Exec(const std::string& command) {
  int rc = libssh2_channel_exec(raw_channel_ptr_, command.c_str());
  if (rc == 0) return ResultType::kSuccess;

  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Unable to perform channel exec, error code %d", rc);
  return ResultType::kError;
}

ResultType Channel::Read(std::string* result, int buffer_size) {
  int rc;
  bool read_at_least_once = false;
  int length_before;
  do {
    length_before = result->length();
    result->resize(length_before + buffer_size);
    rc = libssh2_channel_read(raw_channel_ptr_, result->data() + length_before,
                              buffer_size);

    if (rc == LIBSSH2_ERROR_EAGAIN) {
      result->resize(length_before);
      break;
    }
    if (rc < 0) {
      result->resize(length_before);
      ERROR("Unable to read channel, error code %d", rc);
      return ResultType::kError;
    }

    read_at_least_once = true;
  } while (rc == buffer_size);

  if (rc != LIBSSH2_ERROR_EAGAIN) {
    result->resize(length_before + rc);
  }

  if (read_at_least_once) return ResultType::kSuccess;
  return ResultType::kAgain;
}

ResultType Channel::Write(const std::string& text, int* written_length) {
  CHECK(text.size() >= *written_length);
  int rc =
      libssh2_channel_write(raw_channel_ptr_, text.data() + *written_length,
                            text.length() - *written_length);
  if (rc > 0) {
    *written_length += rc;
    return ResultType::kSuccess;
  }
  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Unable to write to channel, error code %d", rc);
  return ResultType::kError;
}

ResultType Channel::WriteBlocking(const std::string& text) {
  int written_length = 0;
  do {
    ResultType result = Write(text, &written_length);
    if (result == ResultType::kError) return result;
  } while (written_length < text.length());

  return ResultType::kSuccess;
}

ResultType Channel::RequestPty(const std::string& term) {
  int rc = libssh2_channel_request_pty(raw_channel_ptr_, term.c_str());

  if (rc == 0) return ResultType::kSuccess;

  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Unable to request pty, error code %d", rc);
  return ResultType::kError;
}

ResultType Channel::SendEOF() {
  int rc = libssh2_channel_send_eof(raw_channel_ptr_);

  if (rc == 0) return ResultType::kSuccess;

  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Sending EOF to channel resulted in error code %d", rc);
  return ResultType::kError;
}

ResultType Channel::WaitRemoteEOF() {
  int rc = libssh2_channel_wait_eof(raw_channel_ptr_);

  if (rc == 0) return ResultType::kSuccess;

  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Wait for remote EOF failed, error code %d", rc);
  return ResultType::kError;
}

ResultType Channel::Close() {
  int rc = libssh2_channel_close(raw_channel_ptr_);

  if (rc == 0) return ResultType::kSuccess;
  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Channel closing failed with error code %d", rc);
  return ResultType::kError;
}

ResultType Channel::WaitClosed() {
  int rc = libssh2_channel_wait_closed(raw_channel_ptr_);

  if (rc == 0) return ResultType::kSuccess;
  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Wait for closing channel failed, error code %d", rc);
  return ResultType::kError;
}

int Channel::GetExitStatus() {
  return libssh2_channel_get_exit_status(raw_channel_ptr_);
}

bool Channel::GetRemoteEOF() {
  return libssh2_channel_eof(raw_channel_ptr_) == 1;
}

}  // namespace OrbitSsh