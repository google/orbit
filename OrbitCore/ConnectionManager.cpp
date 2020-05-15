//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ConnectionManager.h"

#include <streambuf>

#include "Capture.h"
#include "CoreApp.h"
#include "EventBuffer.h"
#include "OrbitBase/Logging.h"
#include "SamplingProfiler.h"
#include "Serialization.h"
#include "TcpClient.h"
#include "TestRemoteMessages.h"
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
      GTimerManager->Add(timers[i]);
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

  GTcpClient->AddCallback(Msg_ContextSwitches, [=](const Message& msg) {
    uint32_t num_context_switches = msg.m_Size / sizeof(ContextSwitch);
    const ContextSwitch* context_switches =
        static_cast<const ContextSwitch*>(msg.GetData());
    for (uint32_t i = 0; i < num_context_switches; i++) {
      GCoreApp->ProcessContextSwitch(context_switches[i]);
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
