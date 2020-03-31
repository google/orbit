//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <functional>
#include <thread>
#include <unordered_map>

#include "Core.h"
#include "ScopeTimer.h"
#include "TcpEntity.h"

class TcpServer : public TcpEntity {
 public:
  TcpServer();
  ~TcpServer();

  using TcpEntity::Start;
  void Start(unsigned short a_Port);

  void Receive(const Message& a_Message);

  void SendToUiAsync(const std::wstring& a_Message);
  void SendToUiNow(const std::wstring& a_Message);
  void SendToUiAsync(const std::string& a_Message) {
    SendToUiAsync(s2ws(a_Message));
  }
  void SendToUiNow(const std::string& a_Message) {
    SendToUiNow(s2ws(a_Message));
  }

  typedef std::function<void(const std::wstring&)> StrCallback;

  void SetUiCallback(StrCallback a_Callback) { m_UiCallback = a_Callback; }
  void MainThreadTick();

  void Disconnect();
  bool HasConnection();

  bool IsLocalConnection();

  class tcp_server* GetServer() {
    return m_TcpServer;
  }

  void ResetStats();
  std::vector<std::string> GetStats();

 protected:
  class TcpSocket* GetSocket() override final;

 private:
  class tcp_server* m_TcpServer = nullptr;
  std::thread serverThread_;
  void ServerThread();

  StrCallback m_UiCallback;
  moodycamel::ConcurrentQueue<std::wstring> m_UiLockFreeQueue;

  Timer m_StatTimer;
  ULONG64 m_LastNumMessages;
  ULONG64 m_LastNumBytes;
  ULONG64 m_NumReceivedMessages;
  double m_NumMessagesPerSecond;
  double m_BytesPerSecond;
  uint32_t m_MaxTimersAtOnce;
  uint32_t m_NumTimersAtOnce;
  uint32_t m_NumTargetQueuedEntries;
  uint32_t m_NumTargetFlushedEntries;
  uint32_t m_NumTargetFlushedTcpPackets;
  ULONG64 m_NumMessagesFromPreviousSession;
};

extern TcpServer* GTcpServer;
