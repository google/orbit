// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Session.h"

#include <libssh2.h>

#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

Session::Session(LIBSSH2_SESSION* raw_session_ptr)
    : raw_session_ptr_(raw_session_ptr) {}

Session::Session(Session&& other) {
  raw_session_ptr_ = other.raw_session_ptr_;
  other.raw_session_ptr_ = nullptr;
}

Session& Session::operator=(Session&& other) {
  if (this != &other) {
    raw_session_ptr_ = other.raw_session_ptr_;
    other.raw_session_ptr_ = nullptr;
  }
  return *this;
}

std::optional<Session> Session::Create() {
  LIBSSH2_SESSION* raw_session_ptr = libssh2_session_init();

  if (raw_session_ptr == nullptr) {
    ERROR("Unable to initialize ssh session");
    return std::nullopt;
  }

  return Session(raw_session_ptr);
}

Session::~Session() {
  if (raw_session_ptr_ != nullptr) {
    libssh2_session_free(raw_session_ptr_);
  }
}

ResultType Session::Handshake(Socket* socket_ptr) {
  int rc = libssh2_session_handshake(raw_session_ptr_,
                                     socket_ptr->GetFileDescriptor());
  if (rc == 0) return ResultType::kSuccess;
  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Unable to perform session handshake, error code %d", rc);
  return ResultType::kError;
}

ResultType Session::MatchKnownHosts(std::string host, int port,
                                    std::filesystem::path known_hosts_path) {
  LIBSSH2_KNOWNHOSTS* known_hosts = libssh2_knownhost_init(raw_session_ptr_);

  if (known_hosts == nullptr) {
    int last_error_code = libssh2_session_last_errno(raw_session_ptr_);
    ERROR(
        "Unable to initialize ssh known hosts object, last error code "
        "%d",
        last_error_code);
    return ResultType::kError;
  }

  // libssh2 does not support anything else than:
  // LIBSSH2_KNOWNHOST_FILE_OPENSSH
  int amount_hosts =
      libssh2_knownhost_readfile(known_hosts, known_hosts_path.string().c_str(),
                                 LIBSSH2_KNOWNHOST_FILE_OPENSSH);
  if (amount_hosts < 0) {
    ERROR("Unable to read known hosts file, error code: %d, file path %s",
          amount_hosts, known_hosts_path.c_str());
    libssh2_knownhost_free(known_hosts);
    return ResultType::kError;
  }

  size_t fingerprint_length;
  int fingerprint_type;
  const char* fingerprint = libssh2_session_hostkey(
      raw_session_ptr_, &fingerprint_length, &fingerprint_type);
  if (fingerprint == nullptr) {
    int last_error_code = libssh2_session_last_errno(raw_session_ptr_);
    ERROR("Unable to get host fingerprint, last error code: %d",
          last_error_code);
    libssh2_knownhost_free(known_hosts);
    return ResultType::kError;
  }

  struct libssh2_knownhost* known_host;
  int check_result = libssh2_knownhost_checkp(
      known_hosts, host.c_str(), port, fingerprint, fingerprint_length,
      LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW |
          ((fingerprint_type + 1) << LIBSSH2_KNOWNHOST_KEY_SHIFT),
      &known_host);

  if (check_result != LIBSSH2_KNOWNHOST_CHECK_MATCH) {
    ERROR("Host did not match entry in known_host file, error code %d",
          check_result);
    libssh2_knownhost_free(known_hosts);
    return ResultType::kError;
  }

  libssh2_knownhost_free(known_hosts);
  return ResultType::kSuccess;
}

ResultType Session::Authenticate(std::string username,
                                 std::filesystem::path key_path,
                                 std::string pass_phrase) {
  std::filesystem::path public_key_path = key_path;
  public_key_path.replace_filename(key_path.filename().string() + ".pub");

  int rc = libssh2_userauth_publickey_fromfile(
      raw_session_ptr_, username.c_str(), public_key_path.string().c_str(),
      key_path.string().c_str(), pass_phrase.c_str());

  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;
  if (rc == 0) return ResultType::kSuccess;

  ERROR("Unable to perform ssh authentication, error code %d", rc);
  return ResultType::kError;
}

ResultType Session::Disconnect(std::string message) {
  int rc = libssh2_session_disconnect(raw_session_ptr_, message.c_str());
  LOG("Session disconnected");
  if (rc == 0) return ResultType::kSuccess;
  if (rc == LIBSSH2_ERROR_EAGAIN) return ResultType::kAgain;

  ERROR("Session disconnected with erroc code %d", rc);
  return ResultType::kError;
}

void Session::SetBlocking(bool value) {
  libssh2_session_set_blocking(raw_session_ptr_, value);
}

}  // namespace OrbitSsh
