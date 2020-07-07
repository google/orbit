// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TcpClient.h"

#include <thread>

#include "BaseTypes.h"
#include "Core.h"
#include "Log.h"
#include "OrbitBase/Logging.h"
#include "OrbitType.h"
#include "Tcp.h"
#include "absl/strings/str_format.h"

std::unique_ptr<TcpClient> GTcpClient;

//-----------------------------------------------------------------------------
TcpClient::TcpClient(const std::string& a_Host) { Connect(a_Host); }

//-----------------------------------------------------------------------------
TcpClient::~TcpClient() { Stop(); }

//-----------------------------------------------------------------------------
void TcpClient::Connect(const std::string& address) {
  PRINT_FUNC;
  PRINT_VAR(address);
  std::vector<std::string> vec = absl::StrSplit(address, ':');
  if (vec.size() != 2) {
    ERROR("Invalid address string: %s (expected format is 'host:port')",
          address);
    return;
  }
  const std::string& host = vec[0];
  const std::string& port = vec[1];

  socket_.emplace(io_context_);

  asio::ip::tcp::resolver resolver(io_context_);
  asio::ip::tcp::resolver::query query(
      host, port, asio::ip::tcp::resolver::query::canonical_name);
  asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  asio::ip::tcp::resolver::iterator end;

  // Connect to host
  asio::error_code error = asio::error::host_not_found;
  while (error && endpoint_iterator != end) {
    socket_->close();
    auto err = socket_->connect(*endpoint_iterator++, error);
    PRINT_VAR(err.message().c_str());
  }

  if (error) {
    PRINT_VAR(error.message().c_str());
    m_IsValid = false;
    return;
  }

  m_IsValid = true;
}

void TcpClient::Stop() {
  const bool inWorkerThread =
      std::this_thread::get_id() == workerThread_.get_id();

  if (!inWorkerThread && workerThread_.joinable()) {
    io_context_.stop();
    workerThread_.join();
  }

  if (socket_ && socket_->is_open()) {
    socket_->close();
    socket_ = std::nullopt;
  }

  TcpEntity::Stop();
}

//-----------------------------------------------------------------------------
void TcpClient::Start() {
  TcpEntity::Start();

  PRINT_FUNC;
  CHECK(!workerThread_.joinable());
  workerThread_ = std::thread{[this]() { ClientThread(); }};

  std::string msg("Hello from TcpClient");
  this->Send(msg);

  ReadMessage();
}

//-----------------------------------------------------------------------------
void TcpClient::ClientThread() {
  SetCurrentThreadName(L"OrbitTcpClient");
  LOG("io_service started...");
  asio::io_service::work work(io_context_);
  io_context_.run();
  LOG("io_service ended...");
}

//-----------------------------------------------------------------------------
void TcpClient::ReadMessage() {
  asio::async_read(*socket_, asio::buffer(&message_, sizeof(Message)),
                   [this](const asio::error_code& ec, size_t) {
                     if (!ec) {
                       ReadPayload();
                     } else {
                       OnError(ec);
                     }
                   });
}

//-----------------------------------------------------------------------------
void TcpClient::ReadPayload() {
  payload_ = std::vector<char>{};  // payload_ was moved-from. So we have to
                                   // bring it back into a specified state.
  if (message_.m_Size == 0) {
    ReadFooter();
  } else {
    payload_.resize(message_.m_Size);
    asio::async_read(*socket_, asio::buffer(payload_.data(), message_.m_Size),

                     [this](asio::error_code ec, std::size_t /*length*/) {
                       if (!ec) {
                         ReadFooter();
                       } else {
                         OnError(ec);
                       }
                     });
  }
}

//-----------------------------------------------------------------------------
void TcpClient::ReadFooter() {
  OrbitCore::MagicFooterBuffer footer{};
  asio::read(*socket_, asio::buffer(footer.data(), footer.size()));
  CHECK(footer == OrbitCore::GetMagicFooter());
  DecodeMessage(MessageOwner{message_, std::move(payload_)});
  ReadMessage();
}

//-----------------------------------------------------------------------------
void TcpClient::OnError(const std::error_code& ec) {
  if ((ec == asio::error::eof) || (ec == asio::error::connection_reset)) {
    Message msg(Msg_Unload);
    DecodeMessage(MessageOwner{msg, {}});
  }

  PRINT_VAR(ec.message().c_str());
  LOG("Closing socket");
  m_IsValid = false;
  Stop();
}

//-----------------------------------------------------------------------------
void TcpClient::DecodeMessage(MessageOwner&& a_Message) {
  // Don't process messages from previous captures.
  if (a_Message.m_CaptureID == Message::GCaptureID) {
    Callback(std::move(a_Message));
  }
}
