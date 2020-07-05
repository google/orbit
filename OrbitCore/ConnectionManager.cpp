// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConnectionManager.h"

#include <streambuf>

#include "Capture.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "EventBuffer.h"
#include "KeyAndString.h"
#include "OrbitBase/Logging.h"
#include "SamplingProfiler.h"
#include "TcpClient.h"
#include "TidAndThreadName.h"
#include "TimerManager.h"

ConnectionManager::ConnectionManager() : exit_requested_(false) {}

ConnectionManager::~ConnectionManager() { StopThread(); }

void ConnectionManager::StopThread() {
  if (thread_.joinable()) {
    exit_requested_ = true;
    thread_.join();
  }
}

ConnectionManager& ConnectionManager::Get() {
  static ConnectionManager instance;
  return instance;
}

void ConnectionManager::ConnectToRemote(std::string remote_address) {
  remote_address_ = remote_address;
  StopThread();
  SetupClientCallbacks();
  thread_ = std::thread{[this]() { ConnectionThreadWorker(); }};
}

void ConnectionManager::Stop() { exit_requested_ = true; }

void ConnectionManager::SetupClientCallbacks() {
  GTcpClient->AddCallback(Msg_Timers, [=](const Message& msg) {
    uint32_t numTimers = msg.m_Size / sizeof(Timer);
    const Timer* timers = static_cast<const Timer*>(msg.GetData());
    for (uint32_t i = 0; i < numTimers; ++i) {
      GCoreApp->ProcessTimer(timers[i]);
    }
  });

  GTcpClient->AddCallback(Msg_KeysAndStrings, [=](const Message& msg) {
    std::vector<KeyAndString> keys_and_strings;
    DeserializeObjectBinary(msg.GetDataAsString(), keys_and_strings);

    for (const auto& key_and_string : keys_and_strings) {
      GCoreApp->AddKeyAndString(key_and_string.key, key_and_string.str);
    }
  });

  GTcpClient->AddCallback(Msg_LinuxAddressInfos, [=](const Message& msg) {
    std::vector<LinuxAddressInfo> address_infos;
    DeserializeObjectBinary(msg.GetDataAsString(), address_infos);

    for (const auto& address_info : address_infos) {
      GCoreApp->AddAddressInfo(address_info);
    }
  });

  GTcpClient->AddCallback(Msg_SamplingCallstacks, [=](const Message& msg) {
    std::vector<LinuxCallstackEvent> callstacks;
    DeserializeObjectBinary(msg.GetDataAsString(), callstacks);

    for (auto& cs : callstacks) {
      GCoreApp->ProcessSamplingCallStack(cs);
    }
  });

  GTcpClient->AddCallback(
      Msg_SamplingHashedCallstacks, [=](const Message& msg) {
        std::vector<CallstackEvent> callstacks;
        DeserializeObjectBinary(msg.GetDataAsString(), callstacks);

        for (auto& cs : callstacks) {
          GCoreApp->ProcessHashedSamplingCallStack(cs);
        }
      });

  GTcpClient->AddCallback(Msg_ThreadNames, [=](const Message& msg) {
    std::vector<TidAndThreadName> tid_and_names;
    DeserializeObjectBinary(msg.GetDataAsString(), tid_and_names);

    for (const auto& tid_and_name : tid_and_names) {
      GCoreApp->UpdateThreadName(tid_and_name.tid, tid_and_name.thread_name);
    }
  });

  GTcpClient->AddMainThreadCallback(
      Msg_CaptureStopped, [](const Message&) { GCoreApp->OnCaptureStopped(); });
}

void ConnectionManager::ConnectionThreadWorker() {
  while (!exit_requested_) {
    if (!GTcpClient->IsValid()) {
      GTcpClient->Stop();
      GTcpClient->Connect(remote_address_);
      GTcpClient->Start();
    } else {
      // std::string msg("Hello from dev machine");
      // GTcpClient->Send(msg);
    }

    Sleep(2000);
  }
}
