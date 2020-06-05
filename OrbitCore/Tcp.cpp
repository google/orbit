// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "Tcp.h"

#include "Capture.h"
#include "Core.h"
#include "PrintVar.h"
#include "TcpServer.h"

// Based off asio sample
// https://github.com/chriskohlhoff/asio/

tcp_server::~tcp_server() { PRINT_FUNC; }

tcp_server::tcp_server(asio::io_service& io_service, unsigned short port)
    : acceptor_(io_service, tcp::endpoint(make_address("127.0.0.1"), port)) {
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

void handle_writes(const asio::error_code& error, size_t bytes_transferred) {
  std::string errorStr = error.message();
  PRINT_VAR(error.message().c_str());
  PRINT_VAR(bytes_transferred);
}

void TcpConnection::ReadPayload() {
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
  // TODO: A global shouldn't be used here.
  GTcpServer->GetServer()->RegisterConnection(this->shared_from_this());
  GTcpServer->Receive(a_Message);
}
