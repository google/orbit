// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

std::optional<Socket> Socket::Create(int domain, int type, int protocol) {
  WSADATA wsadata;
  int err = WSAStartup(MAKEWORD(2, 0), &wsadata);
  if (err != 0) {
    ERROR("WSAStartup failed with error: %d", err);
  }

  Descriptor descriptor = socket(domain, type, protocol);
  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to create socket");
    return std::nullopt;
  }

  return Socket(descriptor);
}

Socket::~Socket() {
  if (descriptor_ == LIBSSH2_INVALID_SOCKET) return;

  if (closesocket(descriptor_) == 0) {
    descriptor_ = LIBSSH2_INVALID_SOCKET;
    return;
  }

  PrintWithLastError("Socket abnormal close");
}

ResultType Socket::Accept(std::optional<Socket>* new_socket) {
  Descriptor descriptor = accept(descriptor_, NULL, NULL);

  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to accept");
    return ResultType::kError;
  }

  *new_socket = Socket(descriptor);
  return ResultType::kSuccess;
}

void Socket::PrintWithLastError(const std::string& message) {
  LPWSTR error_string = NULL;
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, WSAGetLastError(),
                MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                (LPWSTR)&error_string, 0, NULL);
  ERROR("%s; error: %s", message.c_str(), error_string);
  LocalFree(error_string);
}

ResultType Socket::Shutdown() {
  if (shutdown(descriptor_, SD_BOTH) == 0) return ResultType::kSuccess;

  PrintWithLastError("Socket abnormal shutdown");
  return ResultType::kError;
}

}  // namespace OrbitSsh