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
  GTcpClient->AddMainThreadCallback(Msg_RemotePerf, [=](const Message& a_Msg) {
    PRINT_VAR(a_Msg.m_Size);
    std::string msgStr = a_Msg.GetDataAsString();
    std::istringstream buffer(msgStr);

    Capture::NewSamplingProfiler();
    Capture::GSamplingProfiler->StartCapture();
    Capture::GSamplingProfiler->SetIsLinuxPerf(true);
    Capture::GSamplingProfiler->StopCapture();
    Capture::GSamplingProfiler->ProcessSamples();
    GCoreApp->RefreshCaptureView();
  });

  GTcpClient->AddCallback(Msg_Timers, [=](const Message& a_Msg) {
    uint32_t numTimers = a_Msg.m_Size / sizeof(Timer);
    const Timer* timers = static_cast<const Timer*>(a_Msg.GetData());
    for (uint32_t i = 0; i < numTimers; ++i) {
      GTimerManager->Add(timers[i]);
    }
  });

  GTcpClient->AddCallback(Msg_KeysAndStrings, [=](const Message& a_Msg) {
    std::vector<KeyAndString> keys_and_strings;
    DeserializeObjectBinary(a_Msg.GetDataAsString(), keys_and_strings);

    for (const auto& key_and_string : keys_and_strings) {
      GCoreApp->AddKeyAndString(key_and_string.key, key_and_string.str);
    }
  });

  GTcpClient->AddCallback(Msg_LinuxAddressInfos, [=](const Message& a_Msg) {
    std::vector<LinuxAddressInfo> address_infos;
    DeserializeObjectBinary(a_Msg.GetDataAsString(), address_infos);

    for (const auto& address_info : address_infos) {
      GCoreApp->AddAddressInfo(address_info);
    }
  });

  GTcpClient->AddCallback(Msg_ContextSwitches, [=](const Message& a_Msg) {
    uint32_t num_context_switches = a_Msg.m_Size / sizeof(ContextSwitch);
    const ContextSwitch* context_switches =
        static_cast<const ContextSwitch*>(a_Msg.GetData());
    for (uint32_t i = 0; i < num_context_switches; i++) {
      GCoreApp->ProcessContextSwitch(context_switches[i]);
    }
  });

  GTcpClient->AddCallback(Msg_SamplingCallstacks, [=](const Message& a_Msg) {
    std::vector<LinuxCallstackEvent> callstacks;
    DeserializeObjectBinary(a_Msg.GetDataAsString(), callstacks);

    for (auto& cs : callstacks) {
      GCoreApp->ProcessSamplingCallStack(cs);
    }
  });

  GTcpClient->AddCallback(
      Msg_SamplingHashedCallstacks, [=](const Message& a_Msg) {
        std::vector<CallstackEvent> callstacks;
        DeserializeObjectBinary(a_Msg.GetDataAsString(), callstacks);

        for (auto& cs : callstacks) {
          GCoreApp->ProcessHashedSamplingCallStack(cs);
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
