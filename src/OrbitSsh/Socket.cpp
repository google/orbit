// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Socket.h"

#include <stddef.h>

#ifdef _WIN32
#include <winsock.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <sys/select.h>
#endif

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"

namespace orbit_ssh {

Socket::Socket(Descriptor file_descriptor) : descriptor_(file_descriptor) {}

Socket::Socket(Socket&& other) : descriptor_(other.descriptor_) {
  other.descriptor_ = LIBSSH2_INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) {
  if (this != &other) {
    descriptor_ = other.descriptor_;
    other.descriptor_ = LIBSSH2_INVALID_SOCKET;
  }
  return *this;
}

outcome::result<void> Socket::Connect(const AddrAndPort& addr_and_port, int domain) const {
  sockaddr_in sin{};
  sin.sin_family = domain;
  sin.sin_port = htons(addr_and_port.port);
  sin.sin_addr.s_addr = inet_addr(addr_and_port.addr.c_str());

  if (sin.sin_addr.s_addr == INADDR_NONE) {
    return Error::kInvalidIP;
  }

  const int rc = connect(descriptor_, reinterpret_cast<sockaddr*>(&sin), sizeof(sockaddr_in));

  if (rc != 0) return GetLastError();

  return outcome::success();
}

outcome::result<void> Socket::Bind(const AddrAndPort& addr_and_port, int domain) const {
  sockaddr_in sin{};
  sin.sin_family = domain;
  sin.sin_port = htons(addr_and_port.port);
  sin.sin_addr.s_addr = inet_addr(addr_and_port.addr.c_str());
  if (sin.sin_addr.s_addr == INADDR_NONE) {
    return outcome::failure(Error::kInvalidIP);
  }

  const int result = bind(descriptor_, reinterpret_cast<sockaddr*>(&sin), sizeof(sockaddr_in));

  if (result != 0) return GetLastError();

  return outcome::success();
}

outcome::result<AddrAndPort> Socket::GetSocketAddrAndPort() const {
  struct sockaddr_in sin {};
  socklen_t len = sizeof(sin);
  if (getsockname(descriptor_, reinterpret_cast<sockaddr*>(&sin), &len) == 0) {
    return outcome::success(AddrAndPort{std::string{inet_ntoa(sin.sin_addr)}, ntohs(sin.sin_port)});
  }

  return GetLastError();
}

outcome::result<void> Socket::Listen() const {
  const int rc = listen(descriptor_, 0);

  if (rc != 0) return GetLastError();

  return outcome::success();
}

outcome::result<void> Socket::CanBeRead() const {
  timeval select_timeout{};
  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = 0;

  fd_set socket_set;
  FD_ZERO(&socket_set);
  FD_SET(descriptor_, &socket_set);

  const int result = select(descriptor_ + 1, &socket_set, nullptr, nullptr, &select_timeout);

  if (result < 0) return GetLastError();

  if (result == 0) return Error::kEagain;

  return outcome::success();
}

outcome::result<std::string> Socket::Receive(size_t buffer_size) const {
  OUTCOME_TRY(CanBeRead());

  std::string buffer(buffer_size, '\0');

  const int rc = recv(descriptor_, buffer.data(), buffer.size(), 0);

  if (rc < 0) return GetLastError();

  buffer.resize(rc);
  return buffer;
}

outcome::result<size_t> Socket::Send(std::string_view data) const {
  const int rc = send(descriptor_, data.data(), data.size(), 0);

  if (rc < 0) return GetLastError();

  return outcome::success(static_cast<size_t>(rc));
}

outcome::result<void> Socket::SendBlocking(std::string_view data) {
  do {
    const auto result = Send(data);
    if (!result && !ShouldITryAgain(result)) {
      // A non recoverable error occurred.
      return result.error();
    }
    data = data.substr(result.value());
  } while (!data.empty());

  return outcome::success();
}

outcome::result<void> Socket::WaitDisconnect() const {
  OUTCOME_TRY(CanBeRead());
  OUTCOME_TRY(auto&& data, Receive());

  if (data.empty()) return outcome::success();

  ORBIT_LOG("Received data after sending shutdown on socket (%d bytes)", data.length());
  return GetLastError();
}

}  // namespace orbit_ssh
