// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <optional>
#include <thread>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

TEST(Socket, Create) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);
  ASSERT_TRUE(socket.has_value());
}

TEST(Socket, GetSocketAddrAndPort) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);
  ASSERT_TRUE(socket.has_value());

  // when bound (0 for getting a free port)
  ASSERT_TRUE(socket.value().Bind("127.0.0.1", 0));
  const auto result = socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().addr, "127.0.0.1");
  EXPECT_NE(result.value().port, -1);
}

TEST(Socket, Bind) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);
  ASSERT_TRUE(socket.has_value());

  // invalid ip address
  ASSERT_TRUE(socket.value().Bind("256.0.0.1", 0).has_error());
  ASSERT_TRUE(socket.value().Bind("localhost", 0).has_error());

  // get free port from operating system
  ASSERT_TRUE(socket.value().Bind("127.0.0.1", 0).has_value());

  const auto result = socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().addr, "127.0.0.1");
  EXPECT_NE(result.value().port, -1);

  // cant have two sockets binding to the same address & port
  auto socket_2 = Socket::Create();
  ASSERT_TRUE(socket_2);
  ASSERT_TRUE(socket_2.has_value());
  ASSERT_TRUE(
      socket_2.value().Bind("127.0.0.1", result.value().port).has_error());
}

TEST(Socket, Listen) {
  auto socket = Socket::Create();
  ASSERT_TRUE(socket);
  ASSERT_TRUE(socket.has_value());

  // bind to 127.0.0.1
  ASSERT_TRUE(socket.value().Bind("127.0.0.1", 0));
  const auto result = socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().addr, "127.0.0.1");
  EXPECT_NE(result.value().port, -1);

  // can't call bind again on the same socket
  EXPECT_FALSE(socket.value().Bind("127.0.0.1", 0));

  // bind first then listen
  socket = Socket::Create();  // fresh socket
  ASSERT_TRUE(socket);
  ASSERT_TRUE(socket.has_value());
  ASSERT_TRUE(socket.value().Bind("127.0.0.1", 0));
  ASSERT_TRUE(socket.value().Listen());
}

TEST(Socket, Connect) {
  const std::string ip_address = "127.0.0.1";
  const int port = 0;

  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.has_value());
  ASSERT_TRUE(listen_socket.value().Bind(ip_address, 0));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(addr_and_port);

  // connection should be possible
  auto connect_socket = Socket::Create();
  ASSERT_TRUE(connect_socket);
  ASSERT_TRUE(connect_socket.has_value());
  const auto result = connect_socket.value().Connect(addr_and_port.value());
  ASSERT_TRUE(result);

  // can't listen when already in use
  ASSERT_FALSE(connect_socket.value().Listen());

  // can't connect again when already connected
  ASSERT_FALSE(connect_socket.value().Connect(ip_address, port));
}

TEST(Socket, Accept) {
  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.has_value());
  ASSERT_TRUE(listen_socket.value().Bind("127.0.0.1", 0));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(addr_and_port);

  // setup connect socket
  auto connect_socket = Socket::Create();
  ASSERT_TRUE(connect_socket);
  ASSERT_TRUE(connect_socket.has_value());
  ASSERT_TRUE(connect_socket.value().Connect(addr_and_port.value()));

  // accept should be possible
  auto accepted_socket = listen_socket.value().Accept();
  ASSERT_TRUE(accepted_socket);
}

TEST(Socket, SendAndReceive) {
  // setup listen socket
  auto listen_socket = Socket::Create();
  ASSERT_TRUE(listen_socket);
  ASSERT_TRUE(listen_socket.has_value());
  ASSERT_TRUE(listen_socket.value().Bind("127.0.0.1", 0));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(addr_and_port);

  // setup client socket
  auto client_socket = Socket::Create();
  ASSERT_TRUE(client_socket);
  ASSERT_TRUE(client_socket.has_value());
  ASSERT_TRUE(client_socket.value().Connect(addr_and_port.value()));

  // accept should be possible
  auto server_socket = listen_socket.value().Accept();
  ASSERT_TRUE(server_socket);

  // listen no longer needed
  listen_socket = OrbitSsh::Error::kUnknown;

  // no data available -> receive would block (aka kAgain)
  EXPECT_TRUE(OrbitSsh::shouldITryAgain(server_socket.value().Receive()));
  EXPECT_TRUE(OrbitSsh::shouldITryAgain(client_socket.value().Receive()));

  // Send client -> server
  const std::string_view send_text = "test text";
  EXPECT_TRUE(client_socket.value().SendBlocking(send_text));

  // Even though this is only a local connection, it might take a split second.
  if (OrbitSsh::shouldITryAgain(server_socket.value().CanBeRead())) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  // Receive at server
  const auto result = server_socket.value().Receive();
  ASSERT_TRUE(result);
  EXPECT_EQ(result.value(), send_text);

  // no data available -> receive would block (aka kAgain)
  EXPECT_TRUE(OrbitSsh::shouldITryAgain(server_socket.value().Receive()));
  EXPECT_TRUE(OrbitSsh::shouldITryAgain(client_socket.value().Receive()));

  // Send server -> client
  const std::string_view send_text2 = "test text 2";
  ASSERT_TRUE(server_socket.value().SendBlocking(send_text2));

  // Even though this is only a local connection, it might take a split second.
  if (OrbitSsh::shouldITryAgain(client_socket.value().CanBeRead())) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
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
  ASSERT_TRUE(listen_socket.has_value());
  ASSERT_TRUE(listen_socket.value().Bind("127.0.0.1", 0));
  ASSERT_TRUE(listen_socket.value().Listen());
  const auto addr_and_port = listen_socket.value().GetSocketAddrAndPort();
  ASSERT_TRUE(addr_and_port);

  // setup client socket
  auto client_socket = Socket::Create();
  ASSERT_TRUE(client_socket);
  ASSERT_TRUE(client_socket.has_value());
  ASSERT_TRUE(client_socket.value().Connect(addr_and_port.value()));

  // accept should be possible
  auto server_socket = listen_socket.value().Accept();
  ASSERT_TRUE(server_socket);
  ASSERT_TRUE(server_socket.has_value());

  // no data available -> receive would block (aka kAgain)
  EXPECT_TRUE(OrbitSsh::shouldITryAgain(server_socket.value().Receive()));
  EXPECT_TRUE(OrbitSsh::shouldITryAgain(client_socket.value().Receive()));

  // send shutdown client
  ASSERT_TRUE(client_socket.value().Shutdown());

  // server should have an immidiate result with WaitDisconnect
  ASSERT_TRUE(server_socket.value().WaitDisconnect());
}

}  // namespace OrbitSsh
