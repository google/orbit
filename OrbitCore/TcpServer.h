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

  void StartServer(uint16_t port);

  void Receive(const Message& a_Message);

  void SendToUiAsync(const std::string& a_Message);
  void SendToUiNow(const std::string& a_Message);

  typedef std::function<void(const std::string&)> StrCallback;

  void SetUiCallback(StrCallback a_Callback) { m_UiCallback = a_Callback; }
  void MainThreadTick();

  void Disconnect();
  bool HasConnection();

  bool IsLocalConnection();

  class tcp_server* GetServer() {
    return m_TcpServer;
  }

  uint16_t GetPort() const { return m_Port; }

  void ResetStats();
  std::vector<std::string> GetStats();

 protected:
  class TcpSocket* GetSocket() final;

 private:
  class tcp_server* m_TcpServer = nullptr;
  std::thread serverThread_;
  void ServerThread();

  StrCallback m_UiCallback;
  moodycamel::ConcurrentQueue<std::string> m_UiLockFreeQueue;

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

  uint16_t m_Port;
};

extern std::unique_ptr<TcpServer> GTcpServer;
