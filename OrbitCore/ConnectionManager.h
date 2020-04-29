//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "LinuxTracingBuffer.h"
#include "Message.h"
#include "ProcessUtils.h"
#include "StringManager.h"
#include "TcpEntity.h"

#ifdef __linux__
#include "LinuxTracingHandler.h"
#endif

// TODO: This class is used in both - client and server side,
//  it needs to be reworked and separated into 2 distinct classes
class ConnectionManager {
 public:
  ConnectionManager();
  ~ConnectionManager();
  static ConnectionManager& Get();
  void InitAsService();
  void ConnectToRemote(std::string a_RemoteAddress);
  void SetSelectedFunctionsOnRemote(const Message& a_Msg);
  bool IsService() const { return is_service_; }
  bool IsClient() const { return !is_service_; }
  void StartCaptureAsRemote(uint32_t pid);
  void StopCaptureAsRemote();
  void Stop();
  const ProcessList& GetProcessList() { return process_list_; }

 private:
  void ConnectionThreadWorker();
  void RemoteThreadWorker();
  void ServerCaptureThreadWorker();

  void StopThread();
  void SetupClientCallbacks();
  void SetupServerCallbacks();
  void SetupIntrospection();
  void SendProcesses(TcpEntity* tcp_entity);
  void SendRemoteProcess(TcpEntity* tcp_entity, uint32_t pid);

  ProcessList process_list_;
  LinuxTracingBuffer tracing_buffer_;
#ifdef __linux__
  LinuxTracingHandler tracing_handler_{&tracing_buffer_};
#endif

  std::thread thread_;
  std::thread server_capture_thread_;
  std::string remote_address_;
  std::atomic<bool> exit_requested_;
  bool is_service_;
};
