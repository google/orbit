// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Socket.h"

namespace orbit_ssh {

outcome::result<Socket> Socket::Create(int domain, int type, int protocol) {
  WSADATA wsadata;
  int err = WSAStartup(MAKEWORD(2, 0), &wsadata);
  if (err != 0) {
    ORBIT_ERROR("WSAStartup failed with error: %d", err);
  }

  Descriptor descriptor = socket(domain, type, protocol);
  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to create socket");
    return GetLastError();
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

outcome::result<Socket> Socket::Accept() const {
  Descriptor descriptor = accept(descriptor_, nullptr, nullptr);

  if (descriptor == LIBSSH2_INVALID_SOCKET) {
    PrintWithLastError("Unable to accept");
    return GetLastError();
  }

  return Socket(descriptor);
}

void Socket::PrintWithLastError(std::string_view message) {
  LPWSTR error_string = nullptr;
  FormatMessage(
      FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
      nullptr, WSAGetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(&error_string), 0, nullptr);
  ORBIT_ERROR("%s: %s", message, error_string);
  LocalFree(error_string);
}

outcome::result<void> Socket::Shutdown() const {
  if (shutdown(descriptor_, SD_BOTH) == 0) return outcome::success();

  PrintWithLastError("Socket abnormal shutdown");
  return GetLastError();
}

std::error_code Socket::GetLastError() {
  return std::error_code{::WSAGetLastError(), std::system_category()};
}

}  // namespace orbit_ssh
