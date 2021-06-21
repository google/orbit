// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SERVICE_MEMORY_INFO_HANDLER_H_
#define SERVICE_MEMORY_INFO_HANDLER_H_

#include <absl/container/flat_hash_map.h>

#include "MemoryTracing/MemoryInfoListener.h"
#include "MemoryTracing/MemoryInfoProducer.h"
#include "OrbitBase/Logging.h"
#include "ProducerEventProcessor.h"
#include "capture.pb.h"

namespace orbit_service {

// This class controls the start and stop of the MemoryInfoProducer, and also
// receives the SystemMemoryUsage events from the MemoryInfoProducer.
class MemoryInfoHandler : public orbit_memory_tracing::MemoryInfoListener {
 public:
  explicit MemoryInfoHandler(ProducerEventProcessor* producer_event_processor)
      : producer_event_processor_{producer_event_processor} {
    CHECK(producer_event_processor_ != nullptr);
  }

  ~MemoryInfoHandler() override = default;
  MemoryInfoHandler(const MemoryInfoHandler&) = delete;
  MemoryInfoHandler& operator=(const MemoryInfoHandler&) = delete;
  MemoryInfoHandler(MemoryInfoHandler&&) = delete;
  MemoryInfoHandler& operator=(MemoryInfoHandler&&) = delete;

  void Start(orbit_grpc_protos::CaptureOptions capture_options);
  void Stop();

  void OnSystemMemoryUsage(orbit_grpc_protos::SystemMemoryUsage system_memory_usage) override;
  void OnCGroupMemoryUsage(orbit_grpc_protos::CGroupMemoryUsage cgroup_memory_usage) override;
  void OnProcessMemoryUsage(orbit_grpc_protos::ProcessMemoryUsage process_memory_usage) override;

 private:
  [[nodiscard]] uint64_t GetSamplingWindowId(uint64_t sample_timestamp) const;
  void ProcessMemoryEventWrapperIfReady(uint64_t sampling_window_id);

  ProducerEventProcessor* producer_event_processor_;
  std::unique_ptr<orbit_memory_tracing::MemoryInfoProducer> cgroup_memory_info_producer_;
  std::unique_ptr<orbit_memory_tracing::MemoryInfoProducer> process_memory_info_producer_;
  std::unique_ptr<orbit_memory_tracing::MemoryInfoProducer> system_memory_info_producer_;
  uint64_t sampling_start_timestamp_ns_;
  uint64_t sampling_period_ns_;
  bool enable_cgroup_memory_ = false;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::MemoryEventWrapper> in_progress_wrappers_;
  absl::Mutex in_progress_wrappers_mutex_;
};

}  // namespace orbit_service

#endif  // SERVICE_MEMORY_INFO_HANDLER_H_