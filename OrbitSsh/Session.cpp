// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Session.h"

#include <libssh2.h>

#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/KnownHostsError.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

Session::Session(LIBSSH2_SESSION* raw_session_ptr)
    : raw_session_ptr_(raw_session_ptr, &libssh2_session_free) {}

outcome::result<Session> Session::Create(Context* context) {
  CHECK(context->active());
  LIBSSH2_SESSION* raw_session_ptr = libssh2_session_init();

  if (raw_session_ptr == nullptr) {
    return Error::kFailedCreatingSession;
  }

  return outcome::success(Session{raw_session_ptr});
}

outcome::result<void> Session::Handshake(Socket* socket_ptr) {
  const int rc = libssh2_session_handshake(raw_session_ptr_.get(),
                                           socket_ptr->GetFileDescriptor());
  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

outcome::result<void> Session::MatchKnownHosts(
    const AddrAndPort& addr_and_port,
    const std::filesystem::path& known_hosts_path) {
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

  size_t fingerprint_length = 0;
  int fingerprint_type = 0;
  const char* fingerprint = libssh2_session_hostkey(
      raw_session_ptr_.get(), &fingerprint_length, &fingerprint_type);

  if (fingerprint == nullptr) {
    libssh2_knownhost_free(known_hosts);
    return static_cast<Error>(
        libssh2_session_last_errno(raw_session_ptr_.get()));
  }

  const int check_result = libssh2_knownhost_checkp(
      known_hosts, addr_and_port.addr.c_str(), addr_and_port.port, fingerprint,
      fingerprint_length,
      LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW |
          ((fingerprint_type + 1) << LIBSSH2_KNOWNHOST_KEY_SHIFT),
      nullptr);

  libssh2_knownhost_free(known_hosts);

  if (check_result != LIBSSH2_KNOWNHOST_CHECK_MATCH) {
    return static_cast<KnownHostsError>(check_result);
  }

  return outcome::success();
}

outcome::result<void> Session::Authenticate(
    const std::string& username, const std::filesystem::path& key_path,
    const std::string& pass_phrase) {
  std::filesystem::path public_key_path = key_path;
  public_key_path.replace_filename(key_path.filename().string() + ".pub");

  const int rc = libssh2_userauth_publickey_fromfile(
      raw_session_ptr_.get(), username.c_str(),
      public_key_path.string().c_str(), key_path.string().c_str(),
      pass_phrase.c_str());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

outcome::result<void> Session::Disconnect(const std::string& message) {
  const int rc =
      libssh2_session_disconnect(raw_session_ptr_.get(), message.c_str());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

void Session::SetBlocking(bool value) {
  libssh2_session_set_blocking(raw_session_ptr_.get(), static_cast<int>(value));
}

}  // namespace OrbitSsh
