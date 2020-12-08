// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Session.h"

#include <libssh2.h>

#include <filesystem>
#include <optional>

#include "LibSsh2Utils.h"
#include "OrbitBase/Logging.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Context.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/KnownHostsError.h"
#include "OrbitSsh/Socket.h"

namespace {

void LogFileContents(const std::filesystem::path& file_path) {
  if (!std::filesystem::exists(file_path)) {
    ERROR("Unable to print contents of file \"%s\": File does not exist.", file_path.string());
    return;
  }

  std::ifstream stream{file_path.string()};
  if (!stream.good()) {
    ERROR("Unable to print contents of file \"%s\": Could not open.", file_path.string());
    return;
  }

  LOG("Contents of file \"%s\":\n%s", file_path.string(),
      std::string{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}});
}

}  // namespace

namespace orbit_ssh {

Session::Session(LIBSSH2_SESSION* raw_session_ptr)
    : raw_session_ptr_(raw_session_ptr, &libssh2_session_free) {}

outcome::result<Session> Session::Create(const Context* context) {
  CHECK(context->active());
  LIBSSH2_SESSION* raw_session_ptr = libssh2_session_init();

  if (raw_session_ptr == nullptr) {
    return Error::kFailedCreatingSession;
  }

  return outcome::success(Session{raw_session_ptr});
}

outcome::result<void> Session::Handshake(Socket* socket_ptr) {
  const int rc = libssh2_session_handshake(raw_session_ptr_.get(), socket_ptr->GetFileDescriptor());
  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

outcome::result<void> Session::MatchKnownHosts(const AddrAndPort& addr_and_port,
                                               const std::filesystem::path& known_hosts_path) {
  LIBSSH2_KNOWNHOSTS* known_hosts = libssh2_knownhost_init(raw_session_ptr_.get());

  if (known_hosts == nullptr) {
    LogFileContents(known_hosts_path);
    const auto [last_errno, error_message] = LibSsh2SessionLastError(raw_session_ptr_.get());
    ERROR("libssh2_knownhost_init call failed, last session error message: %s", error_message);
    return static_cast<Error>(last_errno);
  }

  // libssh2 does not support anything else than:
  // LIBSSH2_KNOWNHOST_FILE_OPENSSH
  const int amount_hosts = libssh2_knownhost_readfile(
      known_hosts, known_hosts_path.string().c_str(), LIBSSH2_KNOWNHOST_FILE_OPENSSH);
  if (amount_hosts < 0) {
    LogFileContents(known_hosts_path);
    const auto [unused_errno, error_message] = LibSsh2SessionLastError(raw_session_ptr_.get());
    ERROR(
        "libssh2_knownhost_readfile() call failed. Tried to to read \"%s\". returned error code "
        "was: %d, last session error message: %s",
        known_hosts_path.string(), amount_hosts, error_message);
    libssh2_knownhost_free(known_hosts);
    return static_cast<Error>(amount_hosts);
  }

  size_t fingerprint_length = 0;
  int fingerprint_type = 0;
  const char* fingerprint =
      libssh2_session_hostkey(raw_session_ptr_.get(), &fingerprint_length, &fingerprint_type);

  if (fingerprint == nullptr) {
    LogFileContents(known_hosts_path);
    const auto [last_errno, error_message] = LibSsh2SessionLastError(raw_session_ptr_.get());
    ERROR("libssh2_session_hostkey() failed, last session error message: %s", error_message);
    libssh2_knownhost_free(known_hosts);
    return static_cast<Error>(last_errno);
  }

  const int check_result = libssh2_knownhost_checkp(
      known_hosts, addr_and_port.addr.c_str(), addr_and_port.port, fingerprint, fingerprint_length,
      LIBSSH2_KNOWNHOST_TYPE_PLAIN | LIBSSH2_KNOWNHOST_KEYENC_RAW |
          ((fingerprint_type + 1) << LIBSSH2_KNOWNHOST_KEY_SHIFT),
      nullptr);

  libssh2_knownhost_free(known_hosts);

  if (check_result != LIBSSH2_KNOWNHOST_CHECK_MATCH) {
    LogFileContents(known_hosts_path);
    const auto [unused_errno, error_message] = LibSsh2SessionLastError(raw_session_ptr_.get());
    ERROR(
        "libssh2_knownhost_checkp() call did not produce a match in list of known hosts. Match "
        "result value: %d. Last session error message: %s",
        check_result, error_message);
    return static_cast<KnownHostsError>(check_result);
  }

  return outcome::success();
}

outcome::result<void> Session::Authenticate(const std::string& username,
                                            const std::filesystem::path& key_path,
                                            const std::string& pass_phrase) {
  std::filesystem::path public_key_path = key_path;
  public_key_path.replace_filename(key_path.filename().string() + ".pub");

  const int rc = libssh2_userauth_publickey_fromfile(
      raw_session_ptr_.get(), username.c_str(), public_key_path.string().c_str(),
      key_path.string().c_str(), pass_phrase.c_str());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

outcome::result<void> Session::Disconnect(const std::string& message) {
  const int rc = libssh2_session_disconnect(raw_session_ptr_.get(), message.c_str());

  if (rc < 0) return static_cast<Error>(rc);

  return outcome::success();
}

void Session::SetBlocking(bool value) {
  libssh2_session_set_blocking(raw_session_ptr_.get(), static_cast<int>(value));
}

}  // namespace orbit_ssh
