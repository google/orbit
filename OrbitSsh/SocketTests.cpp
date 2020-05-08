#include <gtest/gtest.h>

#include <memory>
#include <optional>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/ResultType.h"
#include "OrbitSsh/Socket.h"

namespace OrbitSsh {

TEST(Socket, Create) {
  std::optional<Socket> socket = Socket::Create();
  EXPECT_NE(socket, std::nullopt);
}

TEST(Socket, GetSocketAddrAndPort) {
  std::optional<Socket> socket = Socket::Create();
  EXPECT_NE(socket, std::nullopt);

  ResultType result;
  std::string ip_address;
  int port = -1;

  // when bound (0 for getting a free port)
  result = socket->Bind("127.0.0.1", 0);
  EXPECT_EQ(result, ResultType::kSuccess);
  result = socket->GetSocketAddrAndPort(&ip_address, &port);
  EXPECT_EQ(result, ResultType::kSuccess);
  EXPECT_EQ(ip_address, "127.0.0.1");
  EXPECT_NE(port, -1);
}

TEST(Socket, Bind) {
  std::optional<Socket> socket = Socket::Create();
  ASSERT_NE(socket, std::nullopt);

  ResultType result;

  // bad ip address
  result = socket->Bind("256.0.0.1", 0);
  EXPECT_EQ(result, ResultType::kError);
  result = socket->Bind("localhost", 0);
  EXPECT_EQ(result, ResultType::kError);

  // get free port from operating system
  result = socket->Bind("127.0.0.1", 0);
  EXPECT_EQ(result, ResultType::kSuccess);
  std::string ip_address;
  int port = -1;
  result = socket->GetSocketAddrAndPort(&ip_address, &port);
  EXPECT_EQ(result, ResultType::kSuccess);
  EXPECT_EQ(ip_address, "127.0.0.1");
  EXPECT_NE(port, -1);

  // cant have two sockets binding to the same address & port
  std::optional<Socket> socket_2 = Socket::Create();
  ASSERT_NE(socket_2, std::nullopt);
  result = socket_2->Bind(ip_address, port);
  EXPECT_EQ(result, ResultType::kError);
}

TEST(Socket, Listen) {
  std::optional<Socket> socket = Socket::Create();
  ASSERT_NE(socket, std::nullopt);

  ResultType result;
  std::string ip_address;
  int port;

  // bind to 127.0.0.1
  result = socket->Bind("127.0.0.1", 0);
  EXPECT_EQ(result, ResultType::kSuccess);
  result = socket->GetSocketAddrAndPort(&ip_address, &port);
  EXPECT_EQ(result, ResultType::kSuccess);
  EXPECT_EQ(ip_address, "127.0.0.1");

  // can't call bind again on the same socket
  result = socket->Bind("127.0.0.1", 0);
  EXPECT_EQ(result, ResultType::kError);

  // bind first then listen
  socket = std::nullopt;
  socket = Socket::Create();  // fresh socket
  ASSERT_NE(socket, std::nullopt);
  result = socket->Bind("127.0.0.1", 0);
  EXPECT_EQ(result, ResultType::kSuccess);
  result = socket->Listen();
  EXPECT_EQ(result, ResultType::kSuccess);
}

TEST(Socket, Connect) {
  ResultType result;
  std::string ip_address = "127.0.0.1";
  int port = -1;

  // setup listen socket
  std::optional<Socket> listen_socket = Socket::Create();
  ASSERT_NE(listen_socket, std::nullopt);
  result = listen_socket->Bind(ip_address, 0);
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->Listen();
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->GetSocketAddrAndPort(&ip_address, &port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // connection should be possible
  std::optional<Socket> connect_socket = Socket::Create();
  ASSERT_NE(connect_socket, std::nullopt);
  result = connect_socket->Connect(ip_address, port);
  EXPECT_EQ(result, ResultType::kSuccess);

  // can't listen when already in use
  result = connect_socket->Listen();
  EXPECT_EQ(result, ResultType::kError);

  // can't connect again when already connected
  result = connect_socket->Connect(ip_address, port);
  EXPECT_EQ(result, ResultType::kError);
}

TEST(Socket, Accept) {
  ResultType result;
  std::string ip_address = "127.0.0.1";
  int port = -1;

  // setup listen socket
  std::optional<Socket> listen_socket = Socket::Create();
  ASSERT_NE(listen_socket, std::nullopt);
  result = listen_socket->Bind(ip_address, 0);
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->Listen();
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->GetSocketAddrAndPort(&ip_address, &port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // setup connect socket
  std::optional<Socket> connect_socket = Socket::Create();
  ASSERT_NE(connect_socket, std::nullopt);
  result = connect_socket->Connect(ip_address, port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // accept should be possible
  std::optional<Socket> accepted_socket;
  result = listen_socket->Accept(&accepted_socket);
  EXPECT_EQ(result, ResultType::kSuccess);
  EXPECT_NE(accepted_socket, std::nullopt);
}

TEST(Socket, SendAndReceive) {
  ResultType result;
  std::string ip_address = "127.0.0.1";
  int port = -1;

  // setup listen socket
  std::optional<Socket> listen_socket = Socket::Create();
  ASSERT_NE(listen_socket, std::nullopt);
  result = listen_socket->Bind(ip_address, 0);
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->Listen();
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->GetSocketAddrAndPort(&ip_address, &port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // setup client socket
  std::optional<Socket> client_socket = Socket::Create();
  ASSERT_NE(client_socket, std::nullopt);
  result = client_socket->Connect(ip_address, port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // accept should be possible
  std::optional<Socket> server_socket;
  result = listen_socket->Accept(&server_socket);
  ASSERT_EQ(result, ResultType::kSuccess);
  ASSERT_NE(server_socket, std::nullopt);

  // listen no longer needed
  listen_socket = std::nullopt;

  // no data available -> receive would block (aka kAgain)
  std::string text;
  EXPECT_EQ(server_socket->Receive(&text), ResultType::kAgain);
  EXPECT_EQ(client_socket->Receive(&text), ResultType::kAgain);

  // Send client -> server
  std::string send_text = "test text";
  result = client_socket->SendBlocking(send_text);
  EXPECT_EQ(result, ResultType::kSuccess);

  // Receive at server
  std::string receive_text;
  result = server_socket->Receive(&receive_text);
  EXPECT_EQ(result, ResultType::kSuccess);
  EXPECT_EQ(send_text, receive_text);

  // no data available -> receive would block (aka kAgain)
  EXPECT_EQ(server_socket->Receive(&text), ResultType::kAgain);
  EXPECT_EQ(client_socket->Receive(&text), ResultType::kAgain);

  // Send server -> client
  send_text = "test text 2";
  result = server_socket->SendBlocking(send_text);
  EXPECT_EQ(result, ResultType::kSuccess);

  // Receive at client
  receive_text = "";
  result = client_socket->Receive(&receive_text);
  EXPECT_EQ(result, ResultType::kSuccess);
  EXPECT_EQ(send_text, receive_text);
}

TEST(Socket, Shutdown) {
  ResultType result;
  std::string ip_address = "127.0.0.1";
  int port = -1;

  // setup listen socket
  std::optional<Socket> listen_socket = Socket::Create();
  ASSERT_NE(listen_socket, std::nullopt);
  result = listen_socket->Bind(ip_address, 0);
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->Listen();
  ASSERT_EQ(result, ResultType::kSuccess);
  result = listen_socket->GetSocketAddrAndPort(&ip_address, &port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // setup client socket
  std::optional<Socket> client_socket = Socket::Create();
  ASSERT_NE(client_socket, std::nullopt);
  result = client_socket->Connect(ip_address, port);
  ASSERT_EQ(result, ResultType::kSuccess);

  // accept should be possible
  std::optional<Socket> server_socket;
  result = listen_socket->Accept(&server_socket);
  ASSERT_EQ(result, ResultType::kSuccess);
  ASSERT_NE(server_socket, std::nullopt);

  // listen no longer needed
  listen_socket = std::nullopt;

  // no data available -> receive would block (aka kAgain)
  std::string text;
  EXPECT_EQ(server_socket->Receive(&text), ResultType::kAgain);
  EXPECT_EQ(client_socket->Receive(&text), ResultType::kAgain);

  // send shutdown client
  result = client_socket->Shutdown();
  EXPECT_EQ(result, ResultType::kSuccess);

  // server should have an immidiate result with WaitDisconnect
  result = server_socket->WaitDisconnect();
  EXPECT_EQ(result, ResultType::kSuccess);
}

}  // namespace OrbitSsh