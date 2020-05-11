#include "OrbitAsioServer.h"

#include <OrbitLinuxTracing/OrbitTracing.h>

#include "Core.h"
#include "Introspection.h"
#include "TcpServer.h"

OrbitAsioServer::OrbitAsioServer(uint16_t port) {
  // TODO: Don't use the GTcpServer global. Unfortunately, it's needed in
  //  TcpConnection::DecodeMessage.
  GTcpServer = std::make_unique<TcpServer>();
  tcp_server_ = GTcpServer.get();
  tcp_server_->StartServer(port);

  SetupIntrospection();
  SetupServerCallbacks();
  SetupTransactionServices();
}

void OrbitAsioServer::Run(std::atomic<bool>* exit_requested) {
  std::thread process_list_thread{
      [this, exit_requested] { ProcessListThread(exit_requested); }};

  while (!(*exit_requested)) {
    tcp_server_->ProcessMainThreadCallbacks();
    Sleep(16);
  }

  process_list_thread.join();
}

void OrbitAsioServer::SetupIntrospection() {
#if ORBIT_TRACING_ENABLED
  auto handler =
      std::make_unique<orbit::introspection::Handler>(&tracing_buffer_);
  LinuxTracing::SetOrbitTracingHandler(std::move(handler));
#endif
}

void OrbitAsioServer::ProcessListThread(std::atomic<bool>* exit_requested) {
  while (!(*exit_requested)) {
    SendProcessList();
    Sleep(2000);
  }
}

void OrbitAsioServer::SendProcessList() {
  process_list_.Refresh();
  process_list_.UpdateCpuTimes();
  std::string process_data = SerializeObjectHumanReadable(process_list_);
  tcp_server_->Send(Msg_RemoteProcessList, process_data.data(),
                    process_data.size());
}

void OrbitAsioServer::SetupServerCallbacks() {
  tcp_server_->AddMainThreadCallback(
      Msg_RemoteProcessRequest, [this](const Message& msg) {
        auto pid =
            static_cast<uint32_t>(msg.m_Header.m_GenericHeader.m_Address);
        SendProcess(pid);
      });

  tcp_server_->AddMainThreadCallback(
      Msg_RemoteSelectedFunctionsMap,
      [this](const Message& msg) { SetSelectedFunctions(msg); });

  tcp_server_->AddMainThreadCallback(
      Msg_StartCapture, [this](const Message& msg) {
        auto pid =
            static_cast<uint32_t>(msg.m_Header.m_GenericHeader.m_Address);
        StartCapture(pid);
      });

  tcp_server_->AddMainThreadCallback(Msg_StopCapture,
                                     [this](const Message&) { StopCapture(); });
}

void OrbitAsioServer::SendProcess(uint32_t pid) {
  LOG("Sending info on process %d", pid);
  std::shared_ptr<Process> process = process_list_.GetProcess(pid);
  if (process != nullptr) {
    // TODO: Remove this: pid should be part of every message
    //  and all the messages should to be as stateless as possible.
    process->ListModules();
    process->EnumerateThreads();
    std::string process_data = SerializeObjectHumanReadable(*process);
    tcp_server_->Send(Msg_RemoteProcess, process_data.data(),
                      process_data.size());
  }
}

void OrbitAsioServer::SetSelectedFunctions(const Message& message) {
  LOG("Received selected functions");
  DeserializeObjectBinary(message.GetData(), message.GetSize(),
                          selected_functions_);
}

void OrbitAsioServer::StartCapture(uint32_t pid) {
  LOG("Starting capture");
  tracing_handler_.Start(pid, selected_functions_);
  tracing_buffer_thread_ = std::thread{[this] { TracingBufferThread(); }};
}

void OrbitAsioServer::StopCapture() {
  LOG("Stopping capture");
  tracing_handler_.Stop();
  if (tracing_buffer_thread_.joinable()) {
    tracing_buffer_thread_.join();
  }
}

void OrbitAsioServer::SetupTransactionServices() {
  transaction_service_ = std::make_unique<TransactionService>(tcp_server_);
  symbols_service_ = std::make_unique<SymbolsService>(
      &process_list_, transaction_service_.get());
  frame_pointer_validator_service_ =
      std::make_unique<FramePointerValidatorService>(
          &process_list_, transaction_service_.get());
  process_memory_service_ =
      std::make_unique<ProcessMemoryService>(transaction_service_.get());
}

void OrbitAsioServer::TracingBufferThread() {
  while (tracing_handler_.IsStarted()) {
    Sleep(20);

    std::vector<Timer> timers;
    if (tracing_buffer_.ReadAllTimers(&timers)) {
      Message Msg(Msg_Timers);
      tcp_server_->Send(Msg, timers);
    }

    std::vector<LinuxCallstackEvent> callstacks;
    if (tracing_buffer_.ReadAllCallstacks(&callstacks)) {
      std::string message_data = SerializeObjectBinary(callstacks);
      tcp_server_->Send(Msg_SamplingCallstacks, message_data.c_str(),
                        message_data.size());
    }

    std::vector<CallstackEvent> hashed_callstacks;
    if (tracing_buffer_.ReadAllHashedCallstacks(&hashed_callstacks)) {
      std::string message_data = SerializeObjectBinary(hashed_callstacks);
      tcp_server_->Send(Msg_SamplingHashedCallstacks, message_data.c_str(),
                        message_data.size());
    }

    std::vector<ContextSwitch> context_switches;
    if (tracing_buffer_.ReadAllContextSwitches(&context_switches)) {
      Message Msg(Msg_ContextSwitches);
      tcp_server_->Send(Msg, context_switches);
    }

    std::vector<LinuxAddressInfo> address_infos;
    if (tracing_buffer_.ReadAllAddressInfos(&address_infos)) {
      std::string message_data = SerializeObjectBinary(address_infos);
      tcp_server_->Send(Msg_LinuxAddressInfos, message_data.c_str(),
                        message_data.size());
    }

    std::vector<KeyAndString> keys_and_strings;
    if (tracing_buffer_.ReadAllKeysAndStrings(&keys_and_strings)) {
      std::string message_data = SerializeObjectBinary(keys_and_strings);
      tcp_server_->Send(Msg_KeysAndStrings, message_data.c_str(),
                        message_data.size());
    }

    std::vector<TidAndThreadName> tid_and_names;
    if (tracing_buffer_.ReadAllThreadNames(&tid_and_names)) {
      std::string message_data = SerializeObjectBinary(tid_and_names);
      tcp_server_->Send(Msg_ThreadNames, message_data.c_str(),
                        message_data.size());
    }
  }
}
