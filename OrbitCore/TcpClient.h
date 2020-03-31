//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <thread>
#include <vector>

#include "TcpEntity.h"

class TcpClient : public TcpEntity {
 public:
  TcpClient();
  TcpClient(const std::string& a_Host);
  ~TcpClient();

  void Connect(const std::string& a_Host);
  void Start() override;
  void Stop() override;

 protected:
  void ClientThread();
  void ReadMessage();
  void ReadPayload();
  void ReadFooter();
  void DecodeMessage(Message& a_Message);
  void OnError(const std::error_code& ec);
  virtual TcpSocket* GetSocket() override final { return m_TcpSocket; }

 private:
  Message m_Message;
  std::vector<char> m_Payload;
  std::thread workerThread_;
};

extern std::unique_ptr<TcpClient> GTcpClient;
