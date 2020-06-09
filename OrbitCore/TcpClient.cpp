// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// clang-format off
#include "OrbitAsio.h"
// clang-format on

#include "TcpClient.h"

#include <thread>

#include "BaseTypes.h"
#include "Core.h"
#include "Hijacking.h"
#include "Log.h"
#include "OrbitLib.h"
#include "OrbitType.h"
#include "Tcp.h"
#include "absl/strings/str_format.h"
#include "OrbitBase/Logging.h"

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

  m_TcpService->m_IoService = new asio::io_service();
  m_TcpSocket->m_Socket = new tcp::socket(*m_TcpService->m_IoService);

  asio::ip::tcp::socket& socket = *m_TcpSocket->m_Socket;
  asio::ip::tcp::resolver resolver(*m_TcpService->m_IoService);
  asio::ip::tcp::resolver::query query(host, port,
                                       tcp::resolver::query::canonical_name);
  asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  asio::ip::tcp::resolver::iterator end;

  // Connect to host
  asio::error_code error = asio::error::host_not_found;
  while (error && endpoint_iterator != end) {
    socket.close();
    auto err = socket.connect(*endpoint_iterator++, error);
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
    CHECK(m_TcpService);
    CHECK(m_TcpService->m_IoService);
    m_TcpService->m_IoService->stop();
    workerThread_.join();
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
  asio::io_service::work work(*m_TcpService->m_IoService);
  m_TcpService->m_IoService->run();
  LOG("io_service ended...");
}

//-----------------------------------------------------------------------------
void TcpClient::ReadMessage() {
  asio::async_read(*m_TcpSocket->m_Socket,
                   asio::buffer(&m_Message, sizeof(Message)),
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
  if (m_Message.m_Size == 0) {
    m_Message.m_Data = nullptr;
    ReadFooter();
  } else {
    m_Payload.resize(m_Message.m_Size);
    asio::async_read(*m_TcpSocket->m_Socket,
                     asio::buffer(m_Payload.data(), m_Message.m_Size),

                     [this](asio::error_code ec, std::size_t /*length*/) {
                       if (!ec) {
                         m_Message.m_Data = m_Payload.data();
                         ReadFooter();
                       } else {
                         OnError(ec);
                       }
                     });
  }
}

//-----------------------------------------------------------------------------
void TcpClient::ReadFooter() {
  unsigned int footer = 0;
  asio::read(*m_TcpSocket->m_Socket, asio::buffer(&footer, 4));
  assert(footer == MAGIC_FOOT_MSG);
  DecodeMessage(m_Message);
  ReadMessage();
}

//-----------------------------------------------------------------------------
void TcpClient::OnError(const std::error_code& ec) {
  if ((ec == asio::error::eof) || (ec == asio::error::connection_reset)) {
    Message msg(Msg_Unload);
    DecodeMessage(msg);
  }

  PRINT_VAR(ec.message().c_str());
  LOG("Closing socket");
  m_IsValid = false;
  Stop();
}

//-----------------------------------------------------------------------------
void TcpClient::DecodeMessage(Message& a_Message) {
  if (a_Message.m_CaptureID == Message::GCaptureID) {
    Callback(a_Message);
  } else {
    LOG("Received message from previous capture");
  }
}
