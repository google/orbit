// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Logging.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

outcome::result<Socket> Socket::Create(int domain, int type, int protocol) {
  Descriptor descriptor = socket(domain, type, protocol);
  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to create socket");
    return getLastError();
  }

  return Socket(descriptor);
}

Socket::~Socket() noexcept {
  if (descriptor_ == LIBSSH2_INVALID_SOCKET) return;

  if (close(descriptor_) == 0) {
    descriptor_ = LIBSSH2_INVALID_SOCKET;
    return;
  }
  PrintWithLastError("Socket abnormal close");
}

outcome::result<Socket> Socket::Accept() {
  OUTCOME_TRY(CanBeRead());

  const Descriptor descriptor = accept(descriptor_, NULL, NULL);

  if (descriptor < 0) {
    PrintWithLastError("Unable to accept");
    return getLastError();
  }

  return Socket{descriptor};
}

void Socket::PrintWithLastError(const std::string& message) {
  ERROR("%s: %s", message.c_str(), SafeStrerror(errno));
}

outcome::result<void> Socket::Shutdown() {
  if (shutdown(descriptor_, SHUT_RDWR) == 0) return outcome::success();

  PrintWithLastError("Socket abnormal shutdown");
  return getLastError();
}

std::error_code Socket::getLastError() {
  return std::error_code{errno, std::system_category()};
}

}  // namespace OrbitSsh
