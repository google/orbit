// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <thread>
#include <vector>

#include "FramePointerValidatorServiceImpl.h"
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
  ~OrbitAsioServer();
  void LoopTick();

 private:
  void SetupIntrospection();

  void ProcessListThread();

  void SetupServerCallbacks();
  void SetSelectedFunctions(const Message& message);
  void StartCapture(uint32_t pid);
  void StopCapture();

  void SetupTransactionServices();

  void TracingBufferThread();
  void SendBufferedMessages();

  TcpServer* tcp_server_;

  ProcessList process_list_;

  std::unique_ptr<TransactionService> transaction_service_;
  std::unique_ptr<SymbolsService> symbols_service_;
  std::unique_ptr<ProcessMemoryService> process_memory_service_;

  std::vector<std::shared_ptr<Function>> selected_functions_;
  std::thread tracing_buffer_thread_;
  LinuxTracingBuffer tracing_buffer_;
  LinuxTracing::TracingOptions tracing_options_;
  LinuxTracingHandler tracing_handler_{&tracing_buffer_, tracing_options_};

  std::thread process_list_thread_;
  std::atomic<bool> exit_requested_;
};
