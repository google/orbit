//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Tcp.h"

#include "Capture.h"
#include "Core.h"
#include "PrintVar.h"
#include "TcpServer.h"

// Based off asio sample
// https://github.com/chriskohlhoff/asio/

tcp_server::~tcp_server() { PRINT_FUNC; }

tcp_server::tcp_server(asio::io_service& io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(tcp::v4(), port)) {
  start_accept();
}

void tcp_server::Disconnect() {
  PRINT_FUNC;
  connection_ = nullptr;
}

void tcp_server::RegisterConnection(std::shared_ptr<TcpConnection> connection) {
  PRINT_FUNC;
  connection_ = connection;
}

void tcp_server::start_accept() {
  PRINT_FUNC;

  // creates a socket
  auto new_connection = TcpConnection::create(acceptor_.get_io_service());

  // initiates an asynchronous accept operation
  // to wait for a new connection.
  acceptor_.async_accept(*new_connection->GetSocket().m_Socket,
                         std::bind(&tcp_server::handle_accept, this,
                                   new_connection, std::placeholders::_1));
}

// handle_accept() is called when the asynchronous accept operation
// initiated by start_accept() finishes. It services the client request
void tcp_server::handle_accept(std::shared_ptr<TcpConnection> new_connection,
                               const asio::error_code& error) {
  PRINT_FUNC;
  if (!error) {
    PRINT_FUNC;
    connections_set_.insert(new_connection);
    new_connection->start();
  }

  start_accept();
}

void TcpConnection::ReadMessage() {
  asio::async_read(socket_, asio::buffer(&message_, sizeof(Message)),
                   [this](asio::error_code ec, std::size_t /*length*/) {
                     if (!ec) {
                       num_bytes_received_ += sizeof(Message);
                       ReadPayload();
                     } else {
                       PRINT_VAR(ec.message());
                       socket_.close();
                     }
                   }

  );
}

void TcpConnection::ResetStats() { num_bytes_received_ = 0; }

std::vector<std::string> TcpConnection::GetStats() {
  return std::vector<std::string>();
}

bool IsWebSocketHandshakeMessage(Message& message) {
  char* str = reinterpret_cast<char*>(&message);
  return str[0] == 'G' && str[1] == 'E' && str[2] == 'T';
}

void TcpConnection::handle_request_line(asio::error_code ec,
                                        size_t bytes_transferred) {
  if (!ec) {
    num_bytes_received_ += bytes_transferred;

    asio::streambuf::const_buffers_type bufs = stream_buf_.data();
    std::string str(asio::buffers_begin(bufs),
                    asio::buffers_begin(bufs) + bytes_transferred);

    if (absl::StrContains(str, "Sec-WebSocket-Key:")) {
      std::vector<std::string> tokens = Tokenize(str, " ");
      web_socket_key_ = tokens[1];
      ReplaceStringInPlace(web_socket_key_, "\r\n", "");
    }

    stream_buf_.consume(bytes_transferred);

    if (bytes_transferred == 2) {
      SendWebsocketResponse();
    } else {
      asio::async_read_until(
          socket_, stream_buf_, "\r\n",
          std::bind(&TcpConnection::handle_request_line, this,
                    std::placeholders::_1, std::placeholders::_2));
    }
  } else {
    PRINT_VAR(ec.message());
  }
}

void handle_writes(const asio::error_code& error, size_t bytes_transferred) {
  std::string errorStr = error.message();
  PRINT_VAR(error.message().c_str());
  PRINT_VAR(bytes_transferred);
}

void TcpConnection::SendWebsocketResponse() {
  const char* p =
      "HTTP/1.1 101 Switching Protocols\r\nConnection: "
      "upgrade\r\nSec-WebSocket-Accept: ";
  const char* s = "\r\nUpgrade: websocket\r\n\r\n";

  std::string response = p + web_socket_key_ + s;

  asio::async_write(socket_, asio::buffer(response.data(), response.size()),
                    handle_writes);
}

void TcpConnection::ReadWebsocketHandshake() {
  asio::async_read_until(
      socket_, stream_buf_, "\r\n",
      std::bind(&TcpConnection::handle_request_line, this,
                std::placeholders::_1, std::placeholders::_2));
}

void TcpConnection::ReadPayload() {
  if (IsWebSocketHandshakeMessage(message_)) {
    ReadWebsocketHandshake();
    Message msg(Msg_WebSocketHandshake);
    DecodeMessage(msg);
    return;
  }

  if (message_.m_Size == 0) {
    message_.m_Data = nullptr;
    ReadFooter();
  } else {
    payload_.resize(message_.m_Size);
    asio::async_read(
        socket_, asio::buffer(payload_.data(), message_.m_Size),

        [this](asio::error_code ec, std::size_t bytes_transferred) {
          if (!ec) {
            num_bytes_received_ += bytes_transferred;
            message_.m_Data = payload_.data();
            ReadFooter();
          } else {
            PRINT_VAR(ec.message());
            socket_.close();
          }
        });
  }
}

void TcpConnection::ReadFooter() {
  unsigned int footer = 0;
  asio::read(socket_, asio::buffer(&footer, 4));
  assert(footer == MAGIC_FOOT_MSG);
  num_bytes_received_ += 4;
  DecodeMessage(message_);
  ReadMessage();
}

void TcpConnection::DecodeMessage(Message& a_Message) {
  GTcpServer->GetServer()->RegisterConnection(
      this->shared_from_this());  // TODO:
  GTcpServer->Receive(a_Message);
}
