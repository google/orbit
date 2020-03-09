//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "Message.h"
#include "ProcessUtils.h"
#include "TcpEntity.h"

class ConnectionManager {
 public:
  ConnectionManager();
  ~ConnectionManager();
  static ConnectionManager& Get();
  void Init();
  void InitAsService();
  void ConnectToRemote(std::string a_RemoteAddress);
  void SetSelectedFunctionsOnRemote(const Message& a_Msg);
  bool IsService() const { return m_IsService; }
  void StartCaptureAsRemote(uint32_t pid);
  void StopCaptureAsRemote();
  void Stop();

 private:
  void ConnectionThread();
  void RemoteThread();
  void TerminateThread();
  void SetupClientCallbacks();
  void SetupServerCallbacks();
  void SendProcesses(TcpEntity* tcp_entry);
  void SendRemoteProcess(TcpEntity* tcp_entry, uint32_t pid);

  ProcessList process_list_;
  std::unique_ptr<std::thread> m_Thread;
  std::string m_RemoteAddress;
  std::atomic<bool> m_ExitRequested;
  bool m_IsService;
  std::shared_ptr<class BpfTrace> m_BpfTrace;
};
