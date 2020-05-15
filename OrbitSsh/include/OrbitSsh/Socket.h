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
#include <string>

#include "ResultType.h"

namespace OrbitSsh {

// Socket is a wrapper for linux sockets and winsock. It abstracts and supports
// the functionality needed for ssh.
class Socket {
 public:
  using Descriptor = libssh2_socket_t;

  Socket() = delete;
  Socket(const Socket&) = delete;
  Socket& operator=(const Socket&) = delete;
  Socket(Socket&& other);
  Socket& operator=(Socket&& other);
  ~Socket();

  static std::optional<Socket> Create(int domain = AF_INET,
                                      int type = SOCK_STREAM,
                                      int protocol = IPPROTO_TCP);
  static void PrintWithLastError(const std::string& message);

  ResultType Connect(std::string ip_address, int port, int domain = AF_INET);
  ResultType Bind(std::string ip_address, int port, int domain = AF_INET);
  ResultType Listen();
  ResultType Receive(std::string* result, int buffer_size = 0x400);
  ResultType SendBlocking(const std::string& text);
  ResultType GetSocketAddrAndPort(std::string* ip_address, int* port);
  ResultType Accept(std::optional<Socket>* new_socket);

  ResultType Shutdown();
  ResultType WaitDisconnect();
  Descriptor GetFileDescriptor() { return descriptor_; }

  bool CanBeRead();

 private:
  explicit Socket(Descriptor descriptor);
  ResultType Send(const std::string& text, size_t* sent_length);
  Descriptor descriptor_ = LIBSSH2_INVALID_SOCKET;
};

}  // namespace OrbitSsh

#endif  // ORBIT_SSH_SOCKET_H_