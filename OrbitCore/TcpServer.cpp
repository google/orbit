// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TcpServer.h"

#include <thread>

#include "Callstack.h"
#include "Capture.h"
#include "Context.h"
#include "Core.h"
#include "Log.h"
#include "OrbitBase/Logging.h"
#include "OrbitProcess.h"
#include "OrbitUnreal.h"
#include "SamplingProfiler.h"
#include "Tcp.h"
#include "TimerManager.h"
#include "VariableTracing.h"

std::unique_ptr<TcpServer> GTcpServer;

//-----------------------------------------------------------------------------
TcpServer::TcpServer() : m_TcpServer(nullptr) {
  PRINT_FUNC;
  m_LastNumMessages = 0;
  m_LastNumBytes = 0;
  m_NumReceivedMessages = 0;
  m_NumMessagesPerSecond = 0;
  m_BytesPerSecond = 0;
  m_MaxTimersAtOnce = 0;
  m_NumTimersAtOnce = 0;
  m_NumTargetQueuedEntries = 0;
  m_NumTargetFlushedEntries = 0;
  m_NumTargetFlushedTcpPackets = 0;
  m_NumMessagesFromPreviousSession = 0;
}

//-----------------------------------------------------------------------------
TcpServer::~TcpServer() {
  if (serverThread_.joinable()) {
    io_context_.stop();
    serverThread_.join();
  }

  delete m_TcpServer;
}

//-----------------------------------------------------------------------------
void TcpServer::StartServer(uint16_t a_Port) {
  Start();

  PRINT_FUNC;
  m_TcpServer = new tcp_server(io_context_, a_Port);

  serverThread_ = std::thread{[this]() { ServerThread(); }};

  m_StatTimer.Start();
  m_IsValid = true;
  m_Port = a_Port;
}

//-----------------------------------------------------------------------------
void TcpServer::ResetStats() {
  m_NumReceivedMessages = 0;
  if (m_TcpServer != nullptr) {
    m_TcpServer->ResetStats();
  }
}

//-----------------------------------------------------------------------------
std::vector<std::string> TcpServer::GetStats() {
  std::vector<std::string> stats;
  stats.push_back(VAR_TO_STR(m_NumReceivedMessages));
  stats.push_back(VAR_TO_STR(m_NumMessagesPerSecond));

  if (m_TcpServer) {
    std::string bytesRcv = "Capture::GNumBytesReceiced = " +
                           GetPrettySize(m_TcpServer->GetNumBytesReceived()) +
                           "\n";
    stats.push_back(bytesRcv);
  }

  std::string bitRate =
      "Capture::Bitrate = " +
      GetPrettySize(static_cast<uint64_t>(m_BytesPerSecond)) + "/s" + " ( " +
      GetPrettyBitRate(static_cast<uint64_t>(m_BytesPerSecond)) + " )\n";

  stats.push_back(bitRate);
  return stats;
}

//-----------------------------------------------------------------------------
asio::ip::tcp::socket* TcpServer::GetSocket() {
  return m_TcpServer != nullptr ? m_TcpServer->GetSocket() : nullptr;
}

//-----------------------------------------------------------------------------
void TcpServer::Receive(MessageOwner&& message) {
  ++m_NumReceivedMessages;
  Callback(std::move(message));
}

//-----------------------------------------------------------------------------
void TcpServer::SendToUiAsync(const std::string& message) {
  if (m_UiCallback) {
    m_UiLockFreeQueue.enqueue(message);
  }
}

//-----------------------------------------------------------------------------
void TcpServer::SendToUiNow(const std::string& message) {
  if (m_UiCallback) {
    m_UiCallback(message);
  }
}

//-----------------------------------------------------------------------------
void TcpServer::MainThreadTick() {
  std::string msg;
  while (m_UiLockFreeQueue.try_dequeue(msg)) {
    m_UiCallback(msg);
  }

  static double period = 500.0;
  double elapsedTime = m_StatTimer.QueryMillis();
  if (elapsedTime > period) {
    m_NumMessagesPerSecond =
        (m_NumReceivedMessages - m_LastNumMessages) / (elapsedTime * 0.001);
    m_LastNumMessages = m_NumReceivedMessages;

    uint64_t numBytesReceived =
        m_TcpServer ? m_TcpServer->GetNumBytesReceived() : 0;
    m_BytesPerSecond =
        (numBytesReceived - m_LastNumBytes) / (elapsedTime * 0.001);
    m_LastNumBytes = numBytesReceived;
    m_StatTimer.Reset();
  }

  if (!Capture::IsRemote() && Capture::GInjected && Capture::IsCapturing()) {
    const auto socket = GetSocket();
    if (socket == nullptr || !socket->is_open()) {
      Capture::StopCapture();
    }
  }
}

//-----------------------------------------------------------------------------
bool TcpServer::IsLocalConnection() {
  const auto socket = GetSocket();
  if (socket != nullptr) {
    std::string endPoint = socket->remote_endpoint().address().to_string();
    if (endPoint == "127.0.0.1" || ToLower(endPoint) == "localhost") {
      return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
void TcpServer::Disconnect() {
  PRINT_FUNC;
  if (m_TcpServer != nullptr && m_TcpServer->HasConnection()) {
    Message msg(Msg_Unload, 0, nullptr);
    Send(msg);
    m_TcpServer->Disconnect();
  }
}

//-----------------------------------------------------------------------------
bool TcpServer::HasConnection() {
  return m_TcpServer != nullptr && m_TcpServer->HasConnection();
}

//-----------------------------------------------------------------------------
void TcpServer::ServerThread() {
  PRINT_FUNC;
  SetCurrentThreadName(L"TcpServer");
  io_context_.run();
}
