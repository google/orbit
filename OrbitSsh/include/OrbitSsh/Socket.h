// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SOCKET_H_
#define ORBIT_SSH_SOCKET_H_

#ifdef _WIN32
#include <WinSock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#include <libssh2.h>

#include <optional>
#include <outcome.hpp>
#include <string>

#include "OrbitSsh/AddrAndPort.h"

namespace OrbitSsh {

// Socket is a wrapper for linux sockets and winsock. It abstracts and supports
// the functionality needed for ssh.
class Socket {
 public:
  using Descriptor = libssh2_socket_t;

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;

  ~Socket() noexcept;

  static outcome::result<Socket> Create(int domain = AF_INET,
                                        int type = SOCK_STREAM,
                                        int protocol = IPPROTO_TCP);
  static void PrintWithLastError(const std::string& message);

  [[nodiscard]] outcome::result<void> Connect(const AddrAndPort& addr_and_port,
                                              int domain = AF_INET) const;
  [[nodiscard]] outcome::result<void> Bind(const AddrAndPort& addr_and_port,
                                           int domain = AF_INET) const;
  [[nodiscard]] outcome::result<void> Listen() const;
  [[nodiscard]] outcome::result<std::string> Receive(
      size_t buffer_size = 0x400) const;
  [[nodiscard]] outcome::result<void> SendBlocking(std::string_view data);
  [[nodiscard]] outcome::result<AddrAndPort> GetSocketAddrAndPort() const;
  [[nodiscard]] outcome::result<Socket> Accept() const;

  [[nodiscard]] outcome::result<void> Shutdown() const;
  outcome::result<void> WaitDisconnect();
  [[nodiscard]] Descriptor GetFileDescriptor() const { return descriptor_; }

  [[nodiscard]] outcome::result<void> CanBeRead() const;

 private:
  explicit Socket(Descriptor descriptor);

  [[nodiscard]] outcome::result<size_t> Send(std::string_view data) const;

  Descriptor descriptor_ = LIBSSH2_INVALID_SOCKET;
  static std::error_code GetLastError();
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SOCKET_H_
