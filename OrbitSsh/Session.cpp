// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Session.h"

#include <libssh2.h>

#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/KnownHostsError.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

Session::Session(LIBSSH2_SESSION* raw_session_ptr)
    : raw_session_ptr_(raw_session_ptr, &libssh2_session_free) {}

outcome::result<Session> Session::Create(Context*) {
  LIBSSH2_SESSION* raw_session_ptr = libssh2_session_init();

  if (raw_session_ptr == nullptr) {
    return Error::kFailedCreatingSession;
  }

  return outcome::success(Session{raw_session_ptr});
}

outcome::result<void> Session::Handshake(Socket* socket_ptr) {
  const int rc = libssh2_session_handshake(raw_session_ptr_.get(),
                                           socket_ptr->GetFileDescriptor());
  if (rc == 0) {
    return outcome::success();
  } else {
    return static_cast<Error>(rc);
  }
}

outcome::result<void> Session::MatchKnownHosts(
    std::string host, int port, std::filesystem::path known_hosts_path) {
  LIBSSH2_KNOWNHOSTS* known_hosts =
      libssh2_knownhost_init(raw_session_ptr_.get());

  if (known_hosts == nullptr) {
    return static_cast<Error>(
        libssh2_session_last_errno(raw_session_ptr_.get()));
  }

  // libssh2 does not support anything else than:
  // LIBSSH2_KNOWNHOST_FILE_OPENSSH
  const int amount_hosts =
      libssh2_knownhost_readfile(known_hosts, known_hosts_path.string().c_str(),
                                 LIBSSH2_KNOWNHOST_FILE_OPENSSH);
  if (amount_hosts < 0) {
    libssh2_knownhost_free(known_hosts);
    return static_cast<Error>(amount_hosts);
  }

  size_t fingerprint_length;
  int fingerprint_type;
  const char* fingerprint = libssh2_session_hostkey(
      raw_session_ptr_.get(), &fingerprint_length, &fingerprint_type);

  if (fingerprint == nullptr) {
    libssh2_knownhost_free(known_hosts);
    return static_cast<Error>(
        libssh2_session_last_errno(raw_session_ptr_.get()));
  }

  const int check_result = libssh2_knownhost_checkp(
      known_hosts, host.c_str(), port, fingerprint, fingerprint_length,
      LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW |
          ((fingerprint_type + 1) << LIBSSH2_KNOWNHOST_KEY_SHIFT),
      nullptr);

  libssh2_knownhost_free(known_hosts);

  if (check_result != LIBSSH2_KNOWNHOST_CHECK_MATCH) {
    return static_cast<KnownHostsError>(check_result);
  } else {
    return outcome::success();
  }
}

outcome::result<void> Session::Authenticate(std::string username,
                                            std::filesystem::path key_path,
                                            std::string pass_phrase) {
  std::filesystem::path public_key_path = key_path;
  public_key_path.replace_filename(key_path.filename().string() + ".pub");

  const int rc = libssh2_userauth_publickey_fromfile(
      raw_session_ptr_.get(), username.c_str(),
      public_key_path.string().c_str(), key_path.string().c_str(),
      pass_phrase.c_str());

  if (rc == 0) {
    return outcome::success();
  } else {
    return static_cast<Error>(rc);
  }
}

outcome::result<void> Session::Disconnect(std::string message) {
  const int rc =
      libssh2_session_disconnect(raw_session_ptr_.get(), message.c_str());

  if (rc == 0) {
    return outcome::success();
  } else {
    return static_cast<Error>(rc);
  }
}

void Session::SetBlocking(bool value) {
  libssh2_session_set_blocking(raw_session_ptr_.get(), value);
}

}  // namespace OrbitSsh
