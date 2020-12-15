// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_TRACER_THREAD_H_
#define ORBIT_LINUX_TRACING_TRACER_THREAD_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <linux/perf_event.h>
#include <sys/types.h>
#include <tracepoint.pb.h>

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#include "ContextSwitchAndThreadStateVisitor.h"
#include "ContextSwitchManager.h"
#include "Function.h"
#include "GpuTracepointEventProcessor.h"
#include "LinuxTracingUtils.h"
#include "ManualInstrumentationConfig.h"
#include "OrbitLinuxTracing/TracerListener.h"
#include "PerfEvent.h"
#include "PerfEventProcessor.h"
#include "PerfEventRingBuffer.h"
#include "UprobesUnwindingVisitor.h"
#include "capture.pb.h"

namespace LinuxTracing {

class TracerThread {
 public:
  explicit TracerThread(const orbit_grpc_protos::CaptureOptions& capture_options);

  TracerThread(const TracerThread&) = delete;
  TracerThread& operator=(const TracerThread&) = delete;
  TracerThread(TracerThread&&) = delete;
  TracerThread& operator=(TracerThread&&) = delete;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  void Run(const std::shared_ptr<std::atomic<bool>>& exit_requested);

 private:
  static std::optional<uint64_t> ComputeSamplingPeriodNs(double sampling_frequency) {
    double period_ns_dbl = 1'000'000'000 / sampling_frequency;
    if (period_ns_dbl > 0 &&
        period_ns_dbl <= static_cast<double>(std::numeric_limits<uint64_t>::max())) {
      return std::optional<uint64_t>(period_ns_dbl);
    }
    return std::nullopt;
  }

  void InitUprobesEventVisitor();
  bool OpenUserSpaceProbes(const std::vector<int32_t>& cpus);
  bool OpenUprobes(const LinuxTracing::Function& function, const std::vector<int32_t>& cpus,
                   absl::flat_hash_map<int32_t, int>* fds_per_cpu);
  bool OpenUretprobes(const LinuxTracing::Function& function, const std::vector<int32_t>& cpus,
                      absl::flat_hash_map<int32_t, int>* fds_per_cpu);
  bool OpenMmapTask(const std::vector<int32_t>& cpus);
  bool OpenSampling(const std::vector<int32_t>& cpus);

  void AddUprobesFileDescriptors(const absl::flat_hash_map<int32_t, int>& uprobes_fds_per_cpu,
                                 const LinuxTracing::Function& function);

  void AddUretprobesFileDescriptors(const absl::flat_hash_map<int32_t, int>& uretprobes_fds_per_cpu,
                                    const LinuxTracing::Function& function);
  void OpenUserSpaceProbesRingBuffers(
      const absl::flat_hash_map<int32_t, std::vector<int>>& uprobes_uretpobres_fds_per_cpu);

  bool OpenThreadNameTracepoints(const std::vector<int32_t>& cpus);
  void InitContextSwitchAndThreadStateVisitor();
  bool OpenContextSwitchAndThreadStateTracepoints(const std::vector<int32_t>& cpus);

  bool InitGpuTracepointEventProcessor();
  bool OpenGpuTracepoints(const std::vector<int32_t>& cpus);

  bool OpenInstrumentedTracepoints(const std::vector<int32_t>& cpus);

  void ProcessForkEvent(const perf_event_header& header, PerfEventRingBuffer* ring_buffer);
  void ProcessExitEvent(const perf_event_header& header, PerfEventRingBuffer* ring_buffer);
  void ProcessMmapEvent(const perf_event_header& header, PerfEventRingBuffer* ring_buffer);
  void ProcessSampleEvent(const perf_event_header& header, PerfEventRingBuffer* ring_buffer);
  void ProcessLostEvent(const perf_event_header& header, PerfEventRingBuffer* ring_buffer);

  void DeferEvent(std::unique_ptr<PerfEvent> event);
  std::vector<std::unique_ptr<PerfEvent>> ConsumeDeferredEvents();
  void ProcessDeferredEvents();

  void RetrieveThreadNamesSystemWide();
  void RetrieveTidToPidAssociationSystemWide();
  void RetrieveThreadStatesOfTarget();

  void PrintStatsIfTimerElapsed();

  void Reset();

  // Number of records to read consecutively from a perf_event_open ring buffer
  // before switching to another one.
  static constexpr int32_t ROUND_ROBIN_POLLING_BATCH_SIZE = 5;

  // These values are supposed to be large enough to accommodate enough events
  // in case TracerThread::Run's thread is not scheduled for a few tens of
  // milliseconds.
  static constexpr uint64_t UPROBES_RING_BUFFER_SIZE_KB = 8 * 1024;
  static constexpr uint64_t MMAP_TASK_RING_BUFFER_SIZE_KB = 64;
  static constexpr uint64_t SAMPLING_RING_BUFFER_SIZE_KB = 16 * 1024;
  static constexpr uint64_t THREAD_NAMES_RING_BUFFER_SIZE_KB = 64;
  static constexpr uint64_t CONTEXT_SWITCHES_AND_THREAD_STATE_RING_BUFFER_SIZE_KB = 2 * 1024;
  static constexpr uint64_t GPU_TRACING_RING_BUFFER_SIZE_KB = 256;
  static constexpr uint64_t INSTRUMENTED_TRACEPOINTS_RING_BUFFER_SIZE_KB = 8 * 1024;

  static constexpr uint32_t IDLE_TIME_ON_EMPTY_RING_BUFFERS_US = 100;
  static constexpr uint32_t IDLE_TIME_ON_EMPTY_DEFERRED_EVENTS_US = 1000;

  bool trace_context_switches_;
  pid_t target_pid_;
  uint64_t sampling_period_ns_;
  orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method_;
  std::vector<Function> instrumented_functions_;
  ManualInstrumentationConfig manual_instrumentation_config_;
  bool trace_thread_state_;
  bool trace_gpu_driver_;
  std::vector<orbit_grpc_protos::TracepointInfo> instrumented_tracepoints_;

  TracerListener* listener_ = nullptr;

  std::vector<int> tracing_fds_;
  std::vector<PerfEventRingBuffer> ring_buffers_;

  absl::flat_hash_map<uint64_t, const Function*> uprobes_uretprobes_ids_to_function_;
  absl::flat_hash_set<uint64_t> uprobes_ids_;
  absl::flat_hash_set<uint64_t> uretprobes_ids_;
  absl::flat_hash_set<uint64_t> stack_sampling_ids_;
  absl::flat_hash_set<uint64_t> callchain_sampling_ids_;
  absl::flat_hash_set<uint64_t> task_newtask_ids_;
  absl::flat_hash_set<uint64_t> task_rename_ids_;
  absl::flat_hash_set<uint64_t> sched_switch_ids_;
  absl::flat_hash_set<uint64_t> sched_wakeup_ids_;
  absl::flat_hash_set<uint64_t> amdgpu_cs_ioctl_ids_;
  absl::flat_hash_set<uint64_t> amdgpu_sched_run_job_ids_;
  absl::flat_hash_set<uint64_t> dma_fence_signaled_ids_;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::TracepointInfo> ids_to_tracepoint_info_;

  uint64_t effective_capture_start_timestamp_ns_ = 0;

  std::atomic<bool> stop_deferred_thread_ = false;
  std::vector<std::unique_ptr<PerfEvent>> deferred_events_;
  std::mutex deferred_events_mutex_;
  std::unique_ptr<UprobesUnwindingVisitor> uprobes_unwinding_visitor_;
  std::unique_ptr<ContextSwitchAndThreadStateVisitor> context_switch_and_thread_state_visitor_;
  PerfEventProcessor event_processor_;
  std::unique_ptr<GpuTracepointEventProcessor> gpu_event_processor_;

  struct EventStats {
    void Reset() {
      event_count_begin_ns = MonotonicTimestampNs();
      sched_switch_count = 0;
      sample_count = 0;
      uprobes_count = 0;
      gpu_events_count = 0;
      lost_count = 0;
      lost_count_per_buffer.clear();
      discarded_out_of_order_count = 0;
      unwind_error_count = 0;
      discarded_samples_in_uretprobes_count = 0;
    }

    uint64_t event_count_begin_ns = 0;
    uint64_t sched_switch_count = 0;
    uint64_t sample_count = 0;
    uint64_t uprobes_count = 0;
    uint64_t gpu_events_count = 0;
    uint64_t lost_count = 0;
    absl::flat_hash_map<PerfEventRingBuffer*, uint64_t> lost_count_per_buffer{};
    std::atomic<uint64_t> discarded_out_of_order_count = 0;
    std::atomic<uint64_t> unwind_error_count = 0;
    std::atomic<uint64_t> discarded_samples_in_uretprobes_count = 0;
  };

  static constexpr uint64_t EVENT_STATS_WINDOW_S = 5;
  EventStats stats_{};

  static constexpr uint64_t NS_PER_MILLISECOND = 1'000'000;
  static constexpr uint64_t NS_PER_SECOND = 1'000'000'000;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_THREAD_H_
