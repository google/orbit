//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ConnectionManager.h"

#include "Capture.h"
#include "ContextSwitch.h"
#include "CoreApp.h"
#include "EventBuffer.h"
#include "Introspection.h"
#include "KeyAndString.h"
#include "LinuxAddressInfo.h"
#include "LinuxCallstackEvent.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Tracing.h"
#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "Params.h"
#include "ProcessUtils.h"
#include "SamplingProfiler.h"
#include "Serialization.h"
#include "SymbolHelper.h"
#include "SymbolsManager.h"
#include "TcpClient.h"
#include "TcpServer.h"
#include "TestRemoteMessages.h"
#include "TimerManager.h"

#if __linux__
#include "LinuxUtils.h"
#include "OrbitLinuxTracing/OrbitTracing.h"
#endif

#include <fstream>
#include <iostream>
#include <sstream>
#include <streambuf>

ConnectionManager::ConnectionManager()
    : exit_requested_(false), is_service_(false) {}

ConnectionManager::~ConnectionManager() {
  StopThread();
  StopCaptureAsRemote();
}

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

void ConnectionManager::InitAsService() {
#ifdef __linux__
  GParams.m_TrackContextSwitches = true;
#endif

  is_service_ = true;
  SetupIntrospection();
  SetupServerCallbacks();
  thread_ = std::thread{[this]() { RemoteThreadWorker(); }};
}

void ConnectionManager::SetSelectedFunctionsOnRemote(const Message& a_Msg) {
  PRINT_FUNC;
  const void* a_Data = a_Msg.GetData();
  size_t a_Size = a_Msg.m_Size;

  DeserializeObjectBinary(a_Data, a_Size, Capture::GSelectedFunctions);

  // Select the received functions:
  Capture::GSelectedFunctionsMap.clear();
  for (const std::shared_ptr<Function>& function :
       Capture::GSelectedFunctions) {
    // this also adds the function to the map.
    function->Select();
    Capture::GSelectedFunctionsMap[function->GetVirtualAddress()] =
        function.get();
  }
}

void ConnectionManager::ServerCaptureThreadWorker() {
#ifdef __linux__
  while (tracing_handler_.IsStarted()) {
    Sleep(20);

    std::vector<Timer> timers;
    if (tracing_buffer_.ReadAllTimers(&timers)) {
      Message Msg(Msg_Timers);
      GTcpServer->Send(Msg, timers);
    }

    std::vector<LinuxCallstackEvent> callstacks;
    if (tracing_buffer_.ReadAllCallstacks(&callstacks)) {
      std::string message_data = SerializeObjectBinary(callstacks);
      GTcpServer->Send(Msg_SamplingCallstacks, message_data.c_str(),
                       message_data.size());
    }

    std::vector<CallstackEvent> hashed_callstacks;
    if (tracing_buffer_.ReadAllHashedCallstacks(&hashed_callstacks)) {
      std::string message_data = SerializeObjectBinary(hashed_callstacks);
      GTcpServer->Send(Msg_SamplingHashedCallstacks, message_data.c_str(),
                       message_data.size());
    }

    std::vector<ContextSwitch> context_switches;
    if (tracing_buffer_.ReadAllContextSwitches(&context_switches)) {
      Message Msg(Msg_ContextSwitches);
      GTcpServer->Send(Msg, context_switches);
    }

    std::vector<LinuxAddressInfo> address_infos;
    if (tracing_buffer_.ReadAllAddressInfos(&address_infos)) {
      std::string message_data = SerializeObjectBinary(address_infos);
      GTcpServer->Send(Msg_LinuxAddressInfos, message_data.c_str(),
                       message_data.size());
    }

    std::vector<KeyAndString> keys_and_strings;
    if (tracing_buffer_.ReadAllKeysAndStrings(&keys_and_strings)) {
      std::string message_data = SerializeObjectBinary(keys_and_strings);
      GTcpServer->Send(Msg_KeysAndStrings, message_data.c_str(),
                       message_data.size());
    }
  }
#endif
}

void ConnectionManager::SetupIntrospection() {
#if defined(__linux__) && ORBIT_TRACING_ENABLED
  // Setup introspection handler.
  auto handler =
      std::make_unique<orbit::introspection::Handler>(&tracing_buffer_);
  LinuxTracing::SetOrbitTracingHandler(std::move(handler));
#endif
}

void ConnectionManager::StartCaptureAsRemote(uint32_t pid) {
#ifdef __linux__
  PRINT_FUNC;
  tracing_handler_.Start(pid, Capture::GSelectedFunctionsMap);
  server_capture_thread_ =
      std::thread{[this]() { ServerCaptureThreadWorker(); }};
#endif
}

void ConnectionManager::StopCaptureAsRemote() {
#ifdef __linux__
  PRINT_FUNC;
  tracing_handler_.Stop();
  if (server_capture_thread_.joinable()) {
    server_capture_thread_.join();
  }
#endif
}

void ConnectionManager::Stop() { exit_requested_ = true; }

void ConnectionManager::SetupServerCallbacks() {
  GTcpServer->AddMainThreadCallback(
      Msg_RemoteSelectedFunctionsMap,
      [this](const Message& a_Msg) { SetSelectedFunctionsOnRemote(a_Msg); });

  GTcpServer->AddMainThreadCallback(
      Msg_StartCapture, [this](const Message& msg) {
        uint32_t pid =
            static_cast<uint32_t>(msg.m_Header.m_GenericHeader.m_Address);
        StartCaptureAsRemote(pid);
      });

  GTcpServer->AddMainThreadCallback(
      Msg_StopCapture, [this](const Message&) { StopCaptureAsRemote(); });

  GTcpServer->AddMainThreadCallback(
      Msg_RemoteProcessRequest, [this](const Message& msg) {
        uint32_t pid =
            static_cast<uint32_t>(msg.m_Header.m_GenericHeader.m_Address);

        SendRemoteProcess(GTcpServer.get(), pid);
      });
}

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

void ConnectionManager::SendProcesses(TcpEntity* tcp_entity) {
  process_list_.Refresh();
  process_list_.UpdateCpuTimes();
  std::string process_data = SerializeObjectHumanReadable(process_list_);
  tcp_entity->Send(Msg_RemoteProcessList, process_data.data(),
                   process_data.size());
}

void ConnectionManager::SendRemoteProcess(TcpEntity* tcp_entity, uint32_t pid) {
  std::shared_ptr<Process> process = process_list_.GetProcess(pid);
  if (process) {
    // TODO: remove this - pid should be part of every message,
    // and all the messages should to be as stateless as possible.
    Capture::SetTargetProcess(process);
    process->ListModules();
    process->EnumerateThreads();
    std::string process_data = SerializeObjectHumanReadable(*process);
    tcp_entity->Send(Msg_RemoteProcess, process_data.data(),
                     process_data.size());
  }
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

void ConnectionManager::RemoteThreadWorker() {
  while (!exit_requested_) {
    if (GTcpServer && GTcpServer->HasConnection()) {
      SendProcesses(GTcpServer.get());
    }

    Sleep(2000);
  }
}
