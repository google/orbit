// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_INFO_PRODUCER_H_
#define MEMORY_TRACING_MEMORY_INFO_PRODUCER_H_

#include <absl/synchronization/mutex.h>
#include <stdint.h>

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <utility>

#include "GrpcProtos/Constants.h"
#include "MemoryTracing/MemoryInfoListener.h"

namespace orbit_memory_tracing {
using MemoryInfoProducerRunFn = std::function<void(MemoryInfoListener*, int32_t)>;

// This class periodically produces the memory usage information. When the MemoryInfoProducer is
// started, it creates a thread to extract the memory usage information with a sampling interval
// specified in sampling_period_ns_, and send the extracted information to the listener.
class MemoryInfoProducer {
 public:
  explicit MemoryInfoProducer(uint64_t sampling_period_ns, int32_t pid, MemoryInfoProducerRunFn fn)
      : sampling_period_ns_(sampling_period_ns), pid_(pid), producer_run_fn_(std::move(fn)) {}

  MemoryInfoProducer(const MemoryInfoProducer&) = delete;
  MemoryInfoProducer& operator=(const MemoryInfoProducer&) = delete;
  MemoryInfoProducer(MemoryInfoProducer&&) = delete;
  MemoryInfoProducer& operator=(MemoryInfoProducer&&) = delete;

  void SetListener(MemoryInfoListener* listener) { listener_ = listener; }
  void SetThreadName(std::string_view thread_name) { thread_name_ = thread_name; }

  void Start();
  void Stop();

 private:
  void SetExitRequested(bool exit_requested);
  void Run();

  uint64_t sampling_period_ns_;
  MemoryInfoListener* listener_ = nullptr;
  int32_t pid_ = orbit_grpc_protos::kMissingInfo;
  bool exit_requested_ = true;
  absl::Mutex exit_requested_mutex_;
  std::unique_ptr<std::thread> thread_;
  std::string thread_name_ = "MemInfoPr::Run";
  MemoryInfoProducerRunFn producer_run_fn_;
};

std::unique_ptr<MemoryInfoProducer> CreateSystemMemoryInfoProducer(MemoryInfoListener* listener,
                                                                   uint64_t sampling_period_ns,
                                                                   int32_t pid);
std::unique_ptr<MemoryInfoProducer> CreateCGroupMemoryInfoProducer(MemoryInfoListener* listener,
                                                                   uint64_t sampling_period_ns,
                                                                   int32_t pid);
std::unique_ptr<MemoryInfoProducer> CreateProcessMemoryInfoProducer(MemoryInfoListener* listener,
                                                                    uint64_t sampling_period_ns,
                                                                    int32_t pid);

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_INFO_PRODUCER_H_
