#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "FramePointerValidatorService.h"
#include "LinuxTracingBuffer.h"
#include "LinuxTracingHandler.h"
#include "OrbitLinuxTracing/TracingOptions.h"
#include "ProcessMemoryService.h"
#include "ProcessUtils.h"
#include "SymbolsService.h"
#include "TransactionService.h"

class OrbitAsioServer {
 public:
  explicit OrbitAsioServer(uint16_t port,
                           LinuxTracing::TracingOptions tracing_options);
  void Run(std::atomic<bool>* exit_requested);

 private:
  void SetupIntrospection();

  void ProcessListThread(std::atomic<bool>* exit_requested);

  void SetupServerCallbacks();
  void SendProcess(uint32_t pid);
  void SetSelectedFunctions(const Message& message);
  void StartCapture(uint32_t pid);
  void StopCapture();

  void SetupTransactionServices();

  void TracingBufferThread();

  TcpServer* tcp_server_;

  ProcessList process_list_;

  std::unique_ptr<TransactionService> transaction_service_;
  std::unique_ptr<SymbolsService> symbols_service_;
  std::unique_ptr<ProcessMemoryService> process_memory_service_;
  std::unique_ptr<FramePointerValidatorService>
      frame_pointer_validator_service_;

  std::vector<std::shared_ptr<Function>> selected_functions_;
  std::thread tracing_buffer_thread_;
  LinuxTracingBuffer tracing_buffer_;
  LinuxTracing::TracingOptions tracing_options_;
  LinuxTracingHandler tracing_handler_{&tracing_buffer_, tracing_options_};
};
