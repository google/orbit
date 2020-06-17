// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <thread>
#include <vector>

#include "TcpEntity.h"

class TcpClient : public TcpEntity {
 public:
  TcpClient() = default;
  explicit TcpClient(const std::string& a_Host);
  ~TcpClient() override;

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
  TcpSocket* GetSocket() final { return m_TcpSocket.get(); }

 private:
  Message m_Message;
  std::vector<char> m_Payload;
  std::thread workerThread_;
};

extern std::unique_ptr<TcpClient> GTcpClient;
