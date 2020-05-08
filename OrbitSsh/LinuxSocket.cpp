// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

std::optional<Socket> Socket::Create(int domain, int type, int protocol) {
  Descriptor descriptor = socket(domain, type, protocol);
  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to create socket");
    return std::nullopt;
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

ResultType Socket::Accept(std::optional<Socket>* new_socket) {
  if (!CanBeRead()) return ResultType::kAgain;

  Descriptor descriptor = accept(descriptor_, NULL, NULL);

  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to accept");
    return ResultType::kError;
  }

  *new_socket = Socket(descriptor);
  return ResultType::kSuccess;
}

void Socket::PrintWithLastError(const std::string& message) {
  ERROR("%s; error: %s", message.c_str(), strerror(errno));
}

ResultType Socket::Shutdown() {
  if (shutdown(descriptor_, SHUT_RDWR) == 0) return ResultType::kSuccess;

  PrintWithLastError("Socket abnormal shutdown");
  return ResultType::kError;
}

}  // namespace OrbitSsh