// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_TRACER_THREAD_H_
#define LINUX_TRACING_TRACER_THREAD_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <linux/perf_event.h>
#include <sys/types.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <vector>

#include "Function.h"
#include "GpuTracepointVisitor.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "LeafFunctionCallManager.h"
#include "LibunwindstackMaps.h"
#include "LibunwindstackUnwinder.h"
#include "LinuxTracing/Tracer.h"
#include "LinuxTracing/TracerListener.h"
#include "LinuxTracing/UserSpaceInstrumentationAddresses.h"
#include "LostAndDiscardedEventVisitor.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/ThreadUtils.h"
#include "PerfEvent.h"
#include "PerfEventProcessor.h"
#include "PerfEventRingBuffer.h"
#include "SwitchesStatesNamesVisitor.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesReturnAddressManager.h"
#include "UprobesUnwindingVisitor.h"

namespace orbit_linux_tracing {

class TracerImpl : public Tracer {
 public:
  explicit TracerImpl(
      const orbit_grpc_protos::CaptureOptions& capture_options,
      std::unique_ptr<UserSpaceInstrumentationAddresses> user_space_instrumentation_addresses,
      TracerListener* listener);

  TracerImpl(const TracerImpl&) = delete;
  TracerImpl& operator=(const TracerImpl&) = delete;
  TracerImpl(TracerImpl&&) = delete;
  TracerImpl& operator=(TracerImpl&&) = delete;

  void Start() override;
  void Stop() override;

  void ProcessFunctionEntry(const orbit_grpc_protos::FunctionEntry& function_entry) override;
  void ProcessFunctionExit(const orbit_grpc_protos::FunctionExit& function_exit) override;

 private:
  void Run();
  void Startup();
  void Shutdown();
  void ProcessOneRecord(PerfEventRingBuffer* ring_buffer);
  void InitUprobesEventVisitor();
  bool OpenUserSpaceProbes(const std::vector<int32_t>& cpus);
  bool OpenUprobes(const orbit_linux_tracing::Function& function, const std::vector<int32_t>& cpus,
                   absl::flat_hash_map<int32_t, int>* fds_per_cpu);
  bool OpenUretprobes(const orbit_linux_tracing::Function& function,
                      const std::vector<int32_t>& cpus,
                      absl::flat_hash_map<int32_t, int>* fds_per_cpu);
  bool OpenMmapTask(const std::vector<int32_t>& cpus);
  bool OpenSampling(const std::vector<int32_t>& cpus);

  void AddUprobesFileDescriptors(const absl::flat_hash_map<int32_t, int>& uprobes_fds_per_cpu,
                                 const orbit_linux_tracing::Function& function);

  void AddUretprobesFileDescriptors(const absl::flat_hash_map<int32_t, int>& uretprobes_fds_per_cpu,
                                    const orbit_linux_tracing::Function& function);
  void OpenUserSpaceProbesRingBuffers(
      const absl::flat_hash_map<int32_t, std::vector<int>>& uprobes_uretpobres_fds_per_cpu);

  bool OpenThreadNameTracepoints(const std::vector<int32_t>& cpus);
  void InitSwitchesStatesNamesVisitor();
  bool OpenContextSwitchAndThreadStateTracepoints(const std::vector<int32_t>& cpus);

  void InitGpuTracepointEventVisitor();
  bool OpenGpuTracepoints(const std::vector<int32_t>& cpus);

  bool OpenInstrumentedTracepoints(const std::vector<int32_t>& cpus);

  void InitLostAndDiscardedEventVisitor();

  [[nodiscard]] uint64_t ProcessForkEventAndReturnTimestamp(const perf_event_header& header,
                                                            PerfEventRingBuffer* ring_buffer);
  [[nodiscard]] uint64_t ProcessExitEventAndReturnTimestamp(const perf_event_header& header,
                                                            PerfEventRingBuffer* ring_buffer);
  [[nodiscard]] uint64_t ProcessMmapEventAndReturnTimestamp(const perf_event_header& header,
                                                            PerfEventRingBuffer* ring_buffer);
  [[nodiscard]] uint64_t ProcessSampleEventAndReturnTimestamp(const perf_event_header& header,
                                                              PerfEventRingBuffer* ring_buffer);
  [[nodiscard]] uint64_t ProcessLostEventAndReturnTimestamp(const perf_event_header& header,
                                                            PerfEventRingBuffer* ring_buffer);
  [[nodiscard]] uint64_t ProcessThrottleUnthrottleEventAndReturnTimestamp(
      const perf_event_header& header, PerfEventRingBuffer* ring_buffer);

  void DeferEvent(PerfEvent&& event);
  void ProcessDeferredEvents();

  void RetrieveInitialTidToPidAssociationSystemWide();
  void RetrieveInitialThreadStatesOfTarget();

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

  static constexpr uint32_t IDLE_TIME_ON_EMPTY_RING_BUFFERS_US = 5000;
  static constexpr uint32_t IDLE_TIME_ON_EMPTY_DEFERRED_EVENTS_US = 5000;

  bool trace_context_switches_;
  bool introspection_enabled_;
  pid_t target_pid_;
  std::optional<uint64_t> sampling_period_ns_;
  uint16_t stack_dump_size_;
  orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method_;
  std::vector<Function> instrumented_functions_;
  bool trace_thread_state_;
  bool trace_gpu_driver_;
  std::vector<orbit_grpc_protos::TracepointInfo> instrumented_tracepoints_;

  std::unique_ptr<UserSpaceInstrumentationAddresses> user_space_instrumentation_addresses_;

  TracerListener* listener_ = nullptr;

  std::atomic<bool> stop_run_thread_ = true;
  std::thread run_thread_;

  std::vector<int> tracing_fds_;
  std::vector<PerfEventRingBuffer> ring_buffers_;
  absl::flat_hash_map<int, uint64_t> fds_to_last_timestamp_ns_;

  absl::flat_hash_map<uint64_t, uint64_t> uprobes_uretprobes_ids_to_function_id_;
  absl::flat_hash_set<uint64_t> uprobes_ids_;
  absl::flat_hash_set<uint64_t> uprobes_with_args_ids_;
  absl::flat_hash_set<uint64_t> uretprobes_ids_;
  absl::flat_hash_set<uint64_t> uretprobes_with_retval_ids_;
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
  std::vector<PerfEvent> deferred_events_being_buffered_
      ABSL_GUARDED_BY(deferred_events_being_buffered_mutex_);
  absl::Mutex deferred_events_being_buffered_mutex_;
  std::vector<PerfEvent> deferred_events_to_process_;

  UprobesFunctionCallManager function_call_manager_;
  std::optional<UprobesReturnAddressManager> return_address_manager_;
  std::unique_ptr<LibunwindstackMaps> maps_;
  std::unique_ptr<LibunwindstackUnwinder> unwinder_;
  std::unique_ptr<LeafFunctionCallManager> leaf_function_call_manager_;
  std::unique_ptr<UprobesUnwindingVisitor> uprobes_unwinding_visitor_;
  std::unique_ptr<SwitchesStatesNamesVisitor> switches_states_names_visitor_;
  std::unique_ptr<GpuTracepointVisitor> gpu_event_visitor_;
  std::unique_ptr<LostAndDiscardedEventVisitor> lost_and_discarded_event_visitor_;
  PerfEventProcessor event_processor_;

  struct EventStats {
    void Reset() {
      event_count_begin_ns = orbit_base::CaptureTimestampNs();
      sched_switch_count = 0;
      sample_count = 0;
      uprobes_count = 0;
      gpu_events_count = 0;
      lost_count = 0;
      lost_count_per_buffer.clear();
      discarded_out_of_order_count = 0;
      unwind_error_count = 0;
      samples_in_uretprobes_count = 0;
      thread_state_count = 0;
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
    std::atomic<uint64_t> samples_in_uretprobes_count = 0;
    std::atomic<uint64_t> thread_state_count = 0;
  };

  static constexpr uint64_t EVENT_STATS_WINDOW_S = 5;
  EventStats stats_{};

  static constexpr uint64_t NS_PER_SECOND = 1'000'000'000;
};

}  // namespace orbit_linux_tracing

#endif  // LINUX_TRACING_TRACER_THREAD_H_
