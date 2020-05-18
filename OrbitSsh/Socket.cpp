// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Socket.h"

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"

namespace OrbitSsh {

Socket::Socket(Descriptor file_descriptor) : descriptor_(file_descriptor) {}

Socket::Socket(Socket&& other) noexcept {
  descriptor_ = other.descriptor_;
  other.descriptor_ = LIBSSH2_INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) noexcept {
  if (this != &other) {
    descriptor_ = other.descriptor_;
    other.descriptor_ = LIBSSH2_INVALID_SOCKET;
  }
  return *this;
}

outcome::result<void> Socket::Connect(const std::string& ip_address, int port,
                                      int domain) {
  sockaddr_in sin;
  sin.sin_family = domain;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(ip_address.c_str());

  if (sin.sin_addr.s_addr == INADDR_NONE) {
    return Error::kInvalidIP;
  }

  const int rc = connect(descriptor_, reinterpret_cast<sockaddr*>(&sin),
                         sizeof(sockaddr_in));

  if (rc == 0) {
    return outcome::success();
  } else {
    return getLastError();
  }
}

outcome::result<void> Socket::Connect(const AddrAndPort& addrAndPort,
                                      int domain) {
  return Connect(addrAndPort.addr, addrAndPort.port, domain);
}

outcome::result<void> Socket::Bind(const std::string& ip_address, int port,
                                   int domain) {
  sockaddr_in sin;
  sin.sin_family = domain;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(ip_address.c_str());
  if (sin.sin_addr.s_addr == INADDR_NONE) {
    return outcome::failure(Error::kInvalidIP);
  }

  const int result =
      bind(descriptor_, reinterpret_cast<sockaddr*>(&sin), sizeof(sockaddr_in));

  if (result == 0) {
    return outcome::success();
  } else {
    return getLastError();
  }
}

outcome::result<Socket::AddrAndPort> Socket::GetSocketAddrAndPort() {
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(descriptor_, reinterpret_cast<sockaddr*>(&sin), &len) == 0) {
    return outcome::success(
        AddrAndPort{std::string{inet_ntoa(sin.sin_addr)}, ntohs(sin.sin_port)});
  }

  return getLastError();
}

outcome::result<void> Socket::Listen() {
  const int rc = listen(descriptor_, 0);

  if (rc == 0) {
    return outcome::success();
  } else {
    return getLastError();
  }
}

outcome::result<void> Socket::CanBeRead() {
  timeval select_timeout;
  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = 0;

  fd_set socket_set;
  FD_ZERO(&socket_set);
  FD_SET(descriptor_, &socket_set);

  const int result =
      select(descriptor_ + 1, &socket_set, NULL, NULL, &select_timeout);
  if (result > 0) {
    return outcome::success();
  } else if (result == 0) {
    return Error::kEagain;
  } else {
    return getLastError();
  }
}

outcome::result<std::string> Socket::Receive(size_t buffer_size) {
  OUTCOME_TRY(CanBeRead());

  std::string buffer(buffer_size, '\0');

  const int rc = recv(descriptor_, buffer.data(), buffer.size(), 0);

  if (rc < 0) {
    return getLastError();
  } else {
    buffer.resize(rc);
    return buffer;
  }
}

outcome::result<size_t> Socket::Send(std::string_view data) {
  const int rc = send(descriptor_, data.data(), data.size(), 0);

  if (rc > 0) {
    return outcome::success(static_cast<size_t>(rc));
  } else {
    return getLastError();
  }
}

outcome::result<void> Socket::SendBlocking(std::string_view data) {
  do {
    const auto result = Send(data);
    if (!result && !shouldITryAgain(result)) {
      // A non recoverable error occured.
      return result.error();
    }
    data = data.substr(result.value());
  } while (!data.empty());

  return outcome::success();
}

outcome::result<void> Socket::WaitDisconnect() {
  OUTCOME_TRY(CanBeRead());
  OUTCOME_TRY(data, Receive());

  if (data.empty()) {
    return outcome::success();
  }

  LOG("Received data after sending shutdown on socket (%d bytes)",
      data.length());
  return getLastError();
}

}  // namespace OrbitSsh
