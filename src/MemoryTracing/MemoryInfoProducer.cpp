// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracing/MemoryInfoProducer.h"

#include <absl/synchronization/mutex.h>

#include <thread>

#include "MemoryTracingUtils.h"
#include "OrbitBase/ThreadUtils.h"
#include "capture.pb.h"

namespace orbit_memory_tracing {

using orbit_grpc_protos::SystemMemoryUsage;

void MemoryInfoProducer::Start() {
  SetExitRequested(false);
  thread_ = std::make_unique<std::thread>(&MemoryInfoProducer::Run, this);
}

void MemoryInfoProducer::Stop() {
  SetExitRequested(true);
  if (thread_ != nullptr && thread_->joinable()) {
    thread_->join();
  }
  thread_.reset();
}

void MemoryInfoProducer::SetExitRequested(bool exit_requested) {
  absl::MutexLock lock(&exit_requested_mutex_);
  exit_requested_ = exit_requested;
}

}  // namespace orbit_memory_tracing