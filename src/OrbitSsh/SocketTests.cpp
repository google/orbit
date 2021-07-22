// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <string_view>
#include <thread>

#include "OrbitBase/Result.h"
#include "OrbitSsh/AddrAndPort.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Socket.h"

namespace orbit_ssh {

TEST(Socket, Create) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);
}

TEST(Socket, GetSocketAddrAndPort) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);

  // when bound (0 for getting a free port)
  const AddrAndPort localhost_and_port_0{"127.0.0.1", 0};
  ASSERT_TRUE(socket.value().Bind(localhost_and_port_0));
  const auto result = socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().addr, localhost_and_port_0.addr);
  EXPECT_NE(result.value().port, localhost_and_port_0.port);
}

TEST(Socket, Bind) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);

  // invalid ip address
  ASSERT_FALSE(socket.value().Bind(AddrAndPort{"256.0.0.1", 0}));
  ASSERT_FALSE(socket.value().Bind(AddrAndPort{"localhost", 0}, 0));

  // get free port from operating system
  const AddrAndPort localhost_and_port_0{"127.0.0.1", 0};
  ASSERT_TRUE(socket.value().Bind(localhost_and_port_0));

  const auto result = socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().addr, localhost_and_port_0.addr);
  EXPECT_NE(result.value().port, localhost_and_port_0.port);

  // cant have two sockets binding to the same address & port
  auto socket_2 = Socket::Create();
  ASSERT_TRUE(socket_2);
  ASSERT_FALSE(socket_2.value().Bind(result.value()));
}

TEST(Socket, Listen) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);

  // bind to 127.0.0.1
  const AddrAndPort localhost_and_port_0{"127.0.0.1", 0};
  ASSERT_TRUE(socket.value().Bind(localhost_and_port_0));
  const auto result = socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value().addr, localhost_and_port_0.addr);
  EXPECT_NE(result.value().port, localhost_and_port_0.port);

  // can't call bind again on the same socket
  EXPECT_FALSE(socket.value().Bind(localhost_and_port_0));

  // bind first then listen
  socket = Socket::Create();  // fresh socket
  ASSERT_TRUE(socket);
  ASSERT_TRUE(socket.value().Bind(localhost_and_port_0));
  ASSERT_TRUE(socket.value().Listen());
}

TEST(Socket, Connect) {
  const AddrAndPort localhost_and_port_0{"127.0.0.1", 0};

  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.value().Bind(localhost_and_port_0));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto localhost_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(localhost_and_port);
  EXPECT_EQ(localhost_and_port_0.addr, localhost_and_port.value().addr);
  EXPECT_NE(localhost_and_port_0.port, localhost_and_port.value().port);

  // connection should be possible
  auto connect_socket = Socket::Create();
  ASSERT_TRUE(connect_socket);
  const auto connected_addr_and_port = connect_socket.value().Connect(localhost_and_port.value());
  ASSERT_TRUE(connected_addr_and_port);

  // can't listen when already in use
  ASSERT_FALSE(connect_socket.value().Listen());

  // can't connect again when already connected
  ASSERT_FALSE(connect_socket.value().Connect(localhost_and_port_0));
}

TEST(Socket, Accept) {
  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.value().Bind(AddrAndPort{"127.0.0.1", 0}));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto listen_addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(listen_addr_and_port);

  // setup connect socket
  auto connect_socket = Socket::Create();
  ASSERT_TRUE(connect_socket);
  ASSERT_TRUE(connect_socket.value().Connect(listen_addr_and_port.value()));

  // accept should be possible
  auto accepted_socket = listen_socket.value().Accept();
  ASSERT_TRUE(accepted_socket);
}

TEST(Socket, SendAndReceive) {
  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.value().Bind(AddrAndPort{"127.0.0.1", 0}));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto listen_addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(listen_addr_and_port);

  // setup client socket
  auto client_socket = Socket::Create();
  ASSERT_TRUE(client_socket);
  ASSERT_TRUE(client_socket.value().Connect(listen_addr_and_port.value()));

  // accept should be possible
  auto server_socket = listen_socket.value().Accept();
  ASSERT_TRUE(server_socket);

  // listen no longer needed
  listen_socket = orbit_ssh::Error::kUnknown;

  // no data available -> receive would block (aka kAgain)
  EXPECT_TRUE(orbit_ssh::ShouldITryAgain(server_socket.value().Receive()));
  EXPECT_TRUE(orbit_ssh::ShouldITryAgain(client_socket.value().Receive()));

  // Send client -> server
  const std::string_view send_text = "test text";
  EXPECT_TRUE(client_socket.value().SendBlocking(send_text));

  // Even though this is only a local connection, it might take a split second.
  while (orbit_ssh::ShouldITryAgain(server_socket.value().CanBeRead())) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Receive at server
  const auto result = server_socket.value().Receive();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), send_text);

  // no data available -> receive would block (aka kAgain)
  EXPECT_TRUE(orbit_ssh::ShouldITryAgain(server_socket.value().Receive()));
  EXPECT_TRUE(orbit_ssh::ShouldITryAgain(client_socket.value().Receive()));

  // Send server -> client
  const std::string_view send_text2 = "test text 2";
  ASSERT_TRUE(server_socket.value().SendBlocking(send_text2));

  // Even though this is only a local connection, it might take a split second.
  while (orbit_ssh::ShouldITryAgain(client_socket.value().CanBeRead())) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // Receive at client
  const auto result2 = client_socket.value().Receive();
  ASSERT_TRUE(result2);
  EXPECT_EQ(result2.value(), send_text2);
}

TEST(Socket, Shutdown) {
  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.value().Bind(AddrAndPort{"127.0.0.1", 0}));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto listen_addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(listen_addr_and_port);

  // setup client socket
  auto client_socket = Socket::Create();
  ASSERT_TRUE(client_socket);
  ASSERT_TRUE(client_socket.value().Connect(listen_addr_and_port.value()));

  // accept should be possible
  auto server_socket = listen_socket.value().Accept();
  ASSERT_TRUE(server_socket);

  // no data available -> receive would block (aka kAgain)
  EXPECT_TRUE(orbit_ssh::ShouldITryAgain(server_socket.value().Receive()));
  EXPECT_TRUE(orbit_ssh::ShouldITryAgain(client_socket.value().Receive()));

  // send shutdown client
  ASSERT_TRUE(client_socket.value().Shutdown());

  // Even though this is only a local connection, it might take a split second.
  while (orbit_ssh::ShouldITryAgain(server_socket.value().CanBeRead())) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  // server should have a result with WaitDisconnect
  ASSERT_TRUE(server_socket.value().WaitDisconnect());
}

}  // namespace orbit_ssh
