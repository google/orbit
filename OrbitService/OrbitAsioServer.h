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

class OrbitAsioServer {
 public:
  explicit OrbitAsioServer(uint16_t port,
                           LinuxTracing::TracingOptions tracing_options);
  void LoopTick();

 private:
  void SetupIntrospection();

  void SetupServerCallbacks();
  void SetSelectedFunctions(const Message& message);
  void StartCapture(int32_t pid);
  void StopCapture();

  void TracingBufferThread();
  void SendBufferedMessages();

  TcpServer* tcp_server_;

  std::vector<std::shared_ptr<Function>> selected_functions_;
  std::thread tracing_buffer_thread_;
  LinuxTracingBuffer tracing_buffer_;
  LinuxTracing::TracingOptions tracing_options_;
  LinuxTracingHandler tracing_handler_{&tracing_buffer_, tracing_options_};
};
