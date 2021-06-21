// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryInfoHandler.h"

#include "GrpcProtos/Constants.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"

namespace orbit_service {

void MemoryInfoHandler::Start(orbit_grpc_protos::CaptureOptions capture_options) {
  if (!capture_options.collect_memory_info()) return;

  sampling_start_timestamp_ns_ = orbit_base::CaptureTimestampNs();
  sampling_period_ns_ = capture_options.memory_sampling_period_ns();
  enable_cgroup_memory_ = capture_options.enable_cgroup_memory();

  CHECK(system_memory_info_producer_ == nullptr);
  system_memory_info_producer_ = orbit_memory_tracing::CreateSystemMemoryInfoProducer(
      this, capture_options.memory_sampling_period_ns(), capture_options.pid());
  system_memory_info_producer_->Start();

  if (!capture_options.enable_cgroup_memory()) return;

  CHECK(cgroup_memory_info_producer_ == nullptr);
  cgroup_memory_info_producer_ = orbit_memory_tracing::CreateCGroupMemoryInfoProducer(
      this, capture_options.memory_sampling_period_ns(), capture_options.pid());
  cgroup_memory_info_producer_->Start();

  CHECK(process_memory_info_producer_ == nullptr);
  process_memory_info_producer_ = orbit_memory_tracing::CreateProcessMemoryInfoProducer(
      this, capture_options.memory_sampling_period_ns(), capture_options.pid());
  process_memory_info_producer_->Start();
}

void MemoryInfoHandler::Stop() {
  if (system_memory_info_producer_ != nullptr) {
    system_memory_info_producer_->Stop();
    system_memory_info_producer_.reset();
  }

  if (cgroup_memory_info_producer_ != nullptr) {
    cgroup_memory_info_producer_->Stop();
    cgroup_memory_info_producer_.reset();
  }

  if (process_memory_info_producer_ != nullptr) {
    process_memory_info_producer_->Stop();
    process_memory_info_producer_.reset();
  }
}

void MemoryInfoHandler::OnCGroupMemoryUsage(
    orbit_grpc_protos::CGroupMemoryUsage cgroup_memory_usage) {
  uint64_t sampling_window_id = GetSamplingWindowId(cgroup_memory_usage.timestamp_ns());

  absl::MutexLock lock(&in_progress_wrappers_mutex_);
  *in_progress_wrappers_[sampling_window_id].mutable_cgroup_memory_usage() =
      std::move(cgroup_memory_usage);
  ProcessMemoryEventWrapperIfReady(sampling_window_id);
}

void MemoryInfoHandler::OnProcessMemoryUsage(
    orbit_grpc_protos::ProcessMemoryUsage process_memory_usage) {
  uint64_t sampling_window_id = GetSamplingWindowId(process_memory_usage.timestamp_ns());

  absl::MutexLock lock(&in_progress_wrappers_mutex_);
  *in_progress_wrappers_[sampling_window_id].mutable_process_memory_usage() =
      std::move(process_memory_usage);
  ProcessMemoryEventWrapperIfReady(sampling_window_id);
}

void MemoryInfoHandler::OnSystemMemoryUsage(
    orbit_grpc_protos::SystemMemoryUsage system_memory_usage) {
  uint64_t sampling_window_id = GetSamplingWindowId(system_memory_usage.timestamp_ns());

  absl::MutexLock lock(&in_progress_wrappers_mutex_);
  *in_progress_wrappers_[sampling_window_id].mutable_system_memory_usage() =
      std::move(system_memory_usage);
  ProcessMemoryEventWrapperIfReady(sampling_window_id);
}

uint64_t MemoryInfoHandler::GetSamplingWindowId(uint64_t sample_timestamp_ns) const {
  return static_cast<uint64_t>(
      std::round(static_cast<double>(sample_timestamp_ns - sampling_start_timestamp_ns_) /
                 sampling_period_ns_));
}

void MemoryInfoHandler::ProcessMemoryEventWrapperIfReady(uint64_t sampling_window_id) {
  if (!in_progress_wrappers_.contains(sampling_window_id)) return;

  orbit_grpc_protos::MemoryEventWrapper& wrapper = in_progress_wrappers_[sampling_window_id];
  bool wrapper_is_ready = wrapper.has_system_memory_usage();
  if (enable_cgroup_memory_) {
    wrapper_is_ready =
        wrapper_is_ready && wrapper.has_process_memory_usage() && wrapper.has_cgroup_memory_usage();
  }
  if (!wrapper_is_ready) return;

  uint64_t synchronized_timestamp_ns = wrapper.system_memory_usage().timestamp_ns();
  if (enable_cgroup_memory_) {
    synchronized_timestamp_ns = static_cast<uint64_t>(
        (synchronized_timestamp_ns + wrapper.cgroup_memory_usage().timestamp_ns() +
         wrapper.process_memory_usage().timestamp_ns()) /
        3.0);
  }
  wrapper.set_timestamp_ns(synchronized_timestamp_ns);
  orbit_grpc_protos::ProducerCaptureEvent event;
  *event.mutable_memory_event_wrapper() = std::move(wrapper);
  producer_event_processor_->ProcessEvent(orbit_grpc_protos::kMemoryInfoProducerId,
                                          std::move(event));
  in_progress_wrappers_.erase(sampling_window_id);
}

}  // namespace orbit_service
