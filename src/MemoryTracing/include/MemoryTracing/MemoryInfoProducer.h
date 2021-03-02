// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEMORY_TRACING_MEMORY_INFO_PRODUCER_H_
#define MEMORY_TRACING_MEMORY_INFO_PRODUCER_H_

#include <absl/synchronization/mutex.h>

#include <thread>

#include "MemoryTracing/MemoryInfoListener.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {

// This class periodically produces the SystemMemoryUsage information retrieved from /proc/meminfo.
// When the MemoryInfoProducer is started, it creates a thread to extract the memory usage
// information with a sampling interval specified in sampling_period_ns_, and send the extracted
// information to the listener.
class MemoryInfoProducer {
 public:
  explicit MemoryInfoProducer(uint64_t memory_sampling_period_ns)
      : sampling_period_ns_(memory_sampling_period_ns) {}

  MemoryInfoProducer(const MemoryInfoProducer&) = delete;
  MemoryInfoProducer& operator=(const MemoryInfoProducer&) = delete;
  MemoryInfoProducer(MemoryInfoProducer&&) = delete;
  MemoryInfoProducer& operator=(MemoryInfoProducer&&) = delete;

  ~MemoryInfoProducer() { Stop(); }

  void SetListener(MemoryInfoListener* listener) { listener_ = listener; }

  void Start();
  void Stop();

 private:
  void SetExitRequested(bool exit_requested);
  void Run();
  void ProduceSystemMemoryUsageAndSendToListener();

  uint64_t sampling_period_ns_;
  MemoryInfoListener* listener_ = nullptr;
  bool exit_requested_ = true;
  absl::Mutex exit_requested_mutex_;
  std::unique_ptr<std::thread> thread_;
};

}  // namespace orbit_memory_tracing

#endif  // MEMORY_TRACING_MEMORY_INFO_PRODUCER_H_