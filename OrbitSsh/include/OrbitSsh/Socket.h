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
#include <tuple>

namespace OrbitSsh {

// Socket is a wrapper for linux sockets and winsock. It abstracts and supports
// the functionality needed for ssh.
class Socket {
 public:
  using Descriptor = libssh2_socket_t;

  struct AddrAndPort {
    std::string addr;
    int port;

    friend bool operator==(const AddrAndPort& lhs, const AddrAndPort& rhs) {
      return std::tie(lhs.addr, lhs.port) == std::tie(rhs.addr, rhs.port);
    }

    friend bool operator!=(const AddrAndPort& lhs, const AddrAndPort& rhs) {
      return !(lhs == rhs);
    }
  };

  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;

  Socket(Socket&& other) noexcept;
  Socket& operator=(Socket&& other) noexcept;

  ~Socket() noexcept;

  static outcome::result<Socket> Create(int domain = AF_INET,
                                        int type = SOCK_STREAM,
                                        int protocol = IPPROTO_TCP);
  static void PrintWithLastError(const std::string& message);

  outcome::result<void> Connect(const std::string& ip_address, int port,
                                int domain = AF_INET);
  outcome::result<void> Connect(const AddrAndPort& addrAndPort,
                                int domain = AF_INET);
  outcome::result<void> Bind(const std::string& ip_address, int port,
                             int domain = AF_INET);
  outcome::result<void> Listen();
  outcome::result<std::string> Receive(size_t buffer_size = 0x400);
  outcome::result<void> SendBlocking(std::string_view text);
  outcome::result<AddrAndPort> GetSocketAddrAndPort();
  outcome::result<Socket> Accept();

  outcome::result<void> Shutdown();
  outcome::result<void> WaitDisconnect();
  Descriptor GetFileDescriptor() const { return descriptor_; }

  outcome::result<void> CanBeRead();

 private:
  explicit Socket(Descriptor descriptor);
  outcome::result<size_t> Send(std::string_view data);
  Descriptor descriptor_ = LIBSSH2_INVALID_SOCKET;
  static std::error_code getLastError();
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SOCKET_H_
