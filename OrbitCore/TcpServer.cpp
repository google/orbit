// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "TcpServer.h"

#include <thread>

#include "Callstack.h"
#include "Capture.h"
#include "ConnectionManager.h"
#include "Context.h"
#include "Core.h"
#include "Log.h"
#include "OrbitAsio.h"
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
    CHECK(m_TcpService);
    CHECK(m_TcpService->m_IoService);
    m_TcpService->m_IoService->stop();
    serverThread_.join();
  }

  delete m_TcpServer;
}

//-----------------------------------------------------------------------------
void TcpServer::StartServer(uint16_t a_Port) {
  Start();

  PRINT_FUNC;
  m_TcpService = new TcpService();
  m_TcpServer = new tcp_server(*m_TcpService->m_IoService, a_Port);

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
TcpSocket* TcpServer::GetSocket() {
  return m_TcpServer != nullptr ? m_TcpServer->GetSocket() : nullptr;
}

//-----------------------------------------------------------------------------
void TcpServer::Receive(const Message& a_Message) {
  const Message::Header& MessageHeader = a_Message.GetHeader();
  ++m_NumReceivedMessages;

  switch (a_Message.GetType()) {
    case Msg_String: {
      const char* msg = static_cast<const char*>(a_Message.GetData());
      std::cout << msg << std::endl;
      PRINT_VAR(msg);
      break;
    }
    case Msg_Timer: {
      uint32_t numTimers = a_Message.m_Size / sizeof(Timer);
      const Timer* timers = static_cast<const Timer*>(a_Message.GetData());
      for (uint32_t i = 0; i < numTimers; ++i) {
        GTimerManager->Add(timers[i]);
      }

      if (numTimers > m_MaxTimersAtOnce) {
        m_MaxTimersAtOnce = numTimers;
      }
      m_NumTimersAtOnce = numTimers;

      break;
    }
    case Msg_NumQueuedEntries:
      m_NumTargetQueuedEntries =
          *static_cast<const uint32_t*>(a_Message.GetData());
      break;
    case Msg_NumFlushedEntries:
      m_NumTargetFlushedEntries =
          *static_cast<const uint32_t*>(a_Message.GetData());
      break;
    case Msg_NumFlushedItems:
      m_NumTargetFlushedTcpPackets =
          *static_cast<const uint32_t*>(a_Message.GetData());
      break;
    case Msg_NumInstalledHooks:
      Capture::GNumInstalledHooks =
          *static_cast<const uint32_t*>(a_Message.GetData());
      break;
    case Msg_Callstack: {
      const CallStackPOD* callstackPOD =
          static_cast<const CallStackPOD*>(a_Message.GetData());
      CallStack callstack(*callstackPOD);
      Capture::AddCallstack(callstack);
      break;
    }
    case Msg_OrbitZoneName: {
      const OrbitZoneName* zoneName =
          static_cast<const OrbitZoneName*>(a_Message.GetData());
      Capture::RegisterZoneName(zoneName->m_Address, zoneName->m_Data);
      break;
    }
    case Msg_OrbitUnrealObject: {
      const UnrealObjectHeader& header = MessageHeader.m_UnrealObjectHeader;

      std::wstring& objectName = GOrbitUnreal.GetObjectNames()[header.m_Ptr];
      if (header.m_WideStr) {
        objectName = static_cast<const wchar_t*>(a_Message.GetData());
      } else {
        objectName = s2ws(static_cast<const char*>(a_Message.GetData()));
      }

      break;
    }
    case Msg_ThreadInfo: {
      std::string threadName = a_Message.GetDataAsString();
      Capture::GTargetProcess->SetThreadName(a_Message.m_ThreadId, threadName);
      PRINT_VAR(threadName);
      break;
    }
    default: {
      Callback(a_Message);
      break;
    }
  }
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
    TcpSocket* socket = GetSocket();
    if (socket == nullptr || !socket->m_Socket ||
        !socket->m_Socket->is_open()) {
      Capture::StopCapture();
    }
  }
}

//-----------------------------------------------------------------------------
bool TcpServer::IsLocalConnection() {
  TcpSocket* socket = GetSocket();
  if (socket != nullptr && socket->m_Socket) {
    std::string endPoint =
        socket->m_Socket->remote_endpoint().address().to_string();
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
  CHECK(m_TcpService);
  CHECK(m_TcpService->m_IoService);
  m_TcpService->m_IoService->run();
}
