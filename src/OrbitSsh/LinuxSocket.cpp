// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <errno.h>
#include <libssh2.h>
#include <sys/socket.h>
#include <unistd.h>

#include <string_view>
#include <system_error>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "OrbitSsh/Socket.h"

namespace orbit_ssh {

outcome::result<Socket> Socket::Create(int domain, int type, int protocol) {
  Descriptor descriptor = socket(domain, type, protocol);
  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to create socket");
    return GetLastError();
  }

  return Socket(descriptor);
}

Socket::~Socket() {
  if (descriptor_ == LIBSSH2_INVALID_SOCKET) return;

  if (close(descriptor_) == 0) {
    descriptor_ = LIBSSH2_INVALID_SOCKET;
    return;
  }
  PrintWithLastError("Socket abnormal close");
}

outcome::result<Socket> Socket::Accept() const {
  OUTCOME_TRY(CanBeRead());

  const Descriptor descriptor = accept(descriptor_, nullptr, nullptr);

  if (descriptor < 0) {
    PrintWithLastError("Unable to accept");
    return GetLastError();
  }

  return Socket{descriptor};
}

void Socket::PrintWithLastError(std::string_view message) {
  ORBIT_ERROR("%s: %s", message, SafeStrerror(errno));
}

outcome::result<void> Socket::Shutdown() const {
  if (shutdown(descriptor_, SHUT_RDWR) == 0) return outcome::success();

  PrintWithLastError("Socket abnormal shutdown");
  return GetLastError();
}

std::error_code Socket::GetLastError() { return std::error_code{errno, std::system_category()}; }

}  // namespace orbit_ssh
