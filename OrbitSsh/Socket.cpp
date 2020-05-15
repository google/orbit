// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/Socket.h"

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"

namespace OrbitSsh {

Socket::Socket(Descriptor file_descriptor) : descriptor_(file_descriptor) {}

Socket::Socket(Socket&& other) {
  descriptor_ = other.descriptor_;
  other.descriptor_ = LIBSSH2_INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) {
  if (this != &other) {
    descriptor_ = other.descriptor_;
    other.descriptor_ = LIBSSH2_INVALID_SOCKET;
    other.~Socket();
  }
  return *this;
}

ResultType Socket::Connect(std::string ip_address, int port, int domain) {
  sockaddr_in sin;
  sin.sin_family = domain;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(ip_address.c_str());
  if (sin.sin_addr.s_addr == INADDR_NONE) {
    ERROR("Unable to parse ip address");
    return ResultType::kError;
  }

  int rc = connect(descriptor_, reinterpret_cast<sockaddr*>(&sin),
                   sizeof(sockaddr_in));

  if (rc == 0) return ResultType::kSuccess;

  PrintWithLastError("Unable to connect to socket");
  return ResultType::kError;
}

ResultType Socket::Bind(std::string ip_address, int port, int domain) {
  sockaddr_in sin;
  sin.sin_family = domain;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = inet_addr(ip_address.c_str());
  if (sin.sin_addr.s_addr == INADDR_NONE) {
    ERROR("Unable to parse ip address");
    return ResultType::kError;
  }

  if (0 == bind(descriptor_, reinterpret_cast<sockaddr*>(&sin),
                sizeof(sockaddr_in))) {
    return ResultType::kSuccess;
  }

  PrintWithLastError("Error while binding socket");
  return ResultType::kError;
}

ResultType Socket::GetSocketAddrAndPort(std::string* ip_address, int* port) {
  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(descriptor_, reinterpret_cast<sockaddr*>(&sin), &len) == 0) {
    *ip_address = inet_ntoa(sin.sin_addr);
    *port = ntohs(sin.sin_port);
    return ResultType::kSuccess;
  }

  PrintWithLastError("Unable to get port");
  return ResultType::kError;
}

ResultType Socket::Listen() {
  int rc = listen(descriptor_, 0);
  if (rc == 0) return ResultType::kSuccess;

  PrintWithLastError("Unable to listen to socket");
  return ResultType::kError;
}

bool Socket::CanBeRead() {
  timeval select_timeout;
  select_timeout.tv_sec = 0;
  select_timeout.tv_usec = 0;

  fd_set socket_set;
  FD_ZERO(&socket_set);
  FD_SET(descriptor_, &socket_set);

  // TODO error handling here
  int result =
      select(descriptor_ + 1, &socket_set, NULL, NULL, &select_timeout);
  return result > 0;
}

ResultType Socket::Receive(std::string* result, int buffer_size) {
  if (!CanBeRead()) return ResultType::kAgain;

  CHECK(buffer_size > 0);

  int rc;
  int length_before;
  do {
    length_before = result->length();
    result->resize(length_before + buffer_size);
    // rc = recv(descriptor_, buffer_, buffer_size, flags);
    rc = recv(descriptor_, result->data() + length_before, buffer_size, 0);

    if (rc < 0) {
      result->resize(length_before);
      PrintWithLastError("Unable to receive from socket");
      return ResultType::kError;
    }

    // result->append(buffer_, rc);
  } while (rc == buffer_size);

  result->resize(length_before + rc);

  return ResultType::kSuccess;
}

ResultType Socket::Send(const std::string& text, size_t* sent_length) {
  int rc = send(descriptor_, text.data() + *sent_length,
                text.length() - *sent_length, 0);

  if (rc > 0) {
    *sent_length += rc;
    return ResultType::kSuccess;
  }
  PrintWithLastError("Unable to send to socket");
  return ResultType::kError;
}

ResultType Socket::SendBlocking(const std::string& text) {
  size_t sent_length = 0;
  do {
    ResultType result = Send(text, &sent_length);
    if (result == ResultType::kError) return result;
  } while (sent_length < text.length());

  return ResultType::kSuccess;
}

ResultType Socket::WaitDisconnect() {
  if (!CanBeRead()) return ResultType::kAgain;

  std::string text;
  ResultType result = Receive(&text);

  if (result == ResultType::kAgain) return ResultType::kAgain;
  if (result == ResultType::kSuccess && text.empty()) {
    return ResultType::kSuccess;
  }

  LOG("Received data after sending shutdown on socket (%d bytes)",
      text.length());
  return ResultType::kError;
}

}  // namespace OrbitSsh