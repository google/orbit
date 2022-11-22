// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracing/MemoryInfoProducer.h"

#include <absl/synchronization/mutex.h>
#include <absl/time/clock.h>
#include <absl/time/time.h>

#include <thread>

#include "GrpcProtos/capture.pb.h"
#include "MemoryTracing/MemoryTracingUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadUtils.h"

namespace orbit_memory_tracing {

using orbit_grpc_protos::CGroupMemoryUsage;
using orbit_grpc_protos::kMissingInfo;
using orbit_grpc_protos::ProcessMemoryUsage;
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

void MemoryInfoProducer::Run() {
  orbit_base::SetCurrentThreadName(thread_name_.c_str());

  ORBIT_CHECK(listener_ != nullptr);
  ORBIT_CHECK(pid_ != kMissingInfo);

  absl::Time scheduled_time = absl::Now();
  absl::MutexLock lock(&exit_requested_mutex_);
  while (!exit_requested_) {
    producer_run_fn_(listener_, pid_);
    scheduled_time += absl::Nanoseconds(sampling_period_ns_);
    exit_requested_mutex_.AwaitWithDeadline(absl::Condition(&exit_requested_), scheduled_time);
  }
}

std::unique_ptr<MemoryInfoProducer> CreateSystemMemoryInfoProducer(MemoryInfoListener* listener,
                                                                   uint64_t sampling_period_ns,
                                                                   int32_t pid) {
  std::unique_ptr<MemoryInfoProducer> system_memory_info_producer =
      std::make_unique<MemoryInfoProducer>(
          sampling_period_ns, pid, [](MemoryInfoListener* listener, int32_t /*pid*/) {
            ErrorMessageOr<SystemMemoryUsage> system_memory_usage = GetSystemMemoryUsage();
            if (system_memory_usage.has_value()) {
              listener->OnSystemMemoryUsage(system_memory_usage.value());
            }
          });
  system_memory_info_producer->SetListener(listener);
  system_memory_info_producer->SetThreadName("SysMemPr::Run");
  return system_memory_info_producer;
}

std::unique_ptr<MemoryInfoProducer> CreateCGroupMemoryInfoProducer(MemoryInfoListener* listener,
                                                                   uint64_t sampling_period_ns,
                                                                   int32_t pid) {
  std::unique_ptr<MemoryInfoProducer> cgroup_memory_info_producer =
      std::make_unique<MemoryInfoProducer>(
          sampling_period_ns, pid, [](MemoryInfoListener* listener, int32_t pid) {
            ErrorMessageOr<CGroupMemoryUsage> cgroup_memory_usage = GetCGroupMemoryUsage(pid);
            if (cgroup_memory_usage.has_value()) {
              listener->OnCGroupMemoryUsage(cgroup_memory_usage.value());
            }
          });
  cgroup_memory_info_producer->SetListener(listener);
  cgroup_memory_info_producer->SetThreadName("CGrMemPr::Run");
  return cgroup_memory_info_producer;
}

std::unique_ptr<MemoryInfoProducer> CreateProcessMemoryInfoProducer(MemoryInfoListener* listener,
                                                                    uint64_t sampling_period_ns,
                                                                    int32_t pid) {
  std::unique_ptr<MemoryInfoProducer> process_memory_info_producer =
      std::make_unique<MemoryInfoProducer>(
          sampling_period_ns, pid, [](MemoryInfoListener* listener, int32_t pid) {
            ErrorMessageOr<ProcessMemoryUsage> process_memory_usage = GetProcessMemoryUsage(pid);
            if (process_memory_usage.has_value()) {
              listener->OnProcessMemoryUsage(process_memory_usage.value());
            }
          });
  process_memory_info_producer->SetListener(listener);
  process_memory_info_producer->SetThreadName("ProMemPr::Run");
  return process_memory_info_producer;
}

}  // namespace orbit_memory_tracing
