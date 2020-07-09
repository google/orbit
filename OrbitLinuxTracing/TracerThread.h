// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_TRACER_THREAD_H_
#define ORBIT_LINUX_TRACING_TRACER_THREAD_H_

#include <Function.h>
#include <OrbitLinuxTracing/TracerListener.h>
#include <linux/perf_event.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <vector>

#include "ContextSwitchManager.h"
#include "GpuTracepointEventProcessor.h"
#include "PerfEvent.h"
#include "PerfEventProcessor.h"
#include "PerfEventProcessor2.h"
#include "PerfEventReaders.h"
#include "PerfEventRingBuffer.h"
#include "Utils.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "capture.pb.h"

namespace LinuxTracing {

class TracerThread {
 public:
  explicit TracerThread(const CaptureOptions& capture_options);

  TracerThread(const TracerThread&) = delete;
  TracerThread& operator=(const TracerThread&) = delete;
  TracerThread(TracerThread&&) = delete;
  TracerThread& operator=(TracerThread&&) = delete;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  void Run(const std::shared_ptr<std::atomic<bool>>& exit_requested);

 private:
  static std::optional<uint64_t> ComputeSamplingPeriodNs(
      double sampling_frequency) {
    double period_ns_dbl = 1'000'000'000 / sampling_frequency;
    if (period_ns_dbl > 0 &&
        period_ns_dbl <=
            static_cast<double>(std::numeric_limits<uint64_t>::max())) {
      return std::optional<uint64_t>(period_ns_dbl);
    } else {
      return std::nullopt;
    }
  }

  bool OpenContextSwitches(const std::vector<int32_t>& cpus);
  void InitUprobesEventProcessor();
  bool OpenUprobes(const std::vector<int32_t>& cpus);
  bool OpenMmapTask(const std::vector<int32_t>& cpus);
  bool OpenSampling(const std::vector<int32_t>& cpus);

  bool InitGpuTracepointEventProcessor();
  static bool OpenRingBufferForGpuTracepoint(
      const char* tracepoint_category, const char* tracepoint_name, int32_t cpu,
      std::vector<int>* gpu_tracing_fds,
      std::vector<PerfEventRingBuffer>* gpu_ring_buffers);
  bool OpenGpuTracepoints(const std::vector<int32_t>& cpus);

  void ProcessContextSwitchCpuWideEvent(const perf_event_header& header,
                                        PerfEventRingBuffer* ring_buffer);
  void ProcessForkEvent(const perf_event_header& header,
                        PerfEventRingBuffer* ring_buffer);
  void ProcessExitEvent(const perf_event_header& header,
                        PerfEventRingBuffer* ring_buffer);
  void ProcessMmapEvent(const perf_event_header& header,
                        PerfEventRingBuffer* ring_buffer);
  void ProcessSampleEvent(const perf_event_header& header,
                          PerfEventRingBuffer* ring_buffer);
  void ProcessLostEvent(const perf_event_header& header,
                        PerfEventRingBuffer* ring_buffer);

  void DeferEvent(std::unique_ptr<PerfEvent> event);
  std::vector<std::unique_ptr<PerfEvent>> ConsumeDeferredEvents();
  void ProcessDeferredEvents();

  void UpdateThreadNamesIfDelayElapsed();

  void PrintStatsIfTimerElapsed();

  void Reset();

  // Number of records to read consecutively from a perf_event_open ring buffer
  // before switching to another one.
  static constexpr int32_t ROUND_ROBIN_POLLING_BATCH_SIZE = 5;

  // These values are supposed to be large enough to accommodate enough events
  // in case TracerThread::Run's thread is not scheduled for a few tens of
  // milliseconds.
  static constexpr uint64_t CONTEXT_SWITCHES_RING_BUFFER_SIZE_KB = 2 * 1024;
  static constexpr uint64_t UPROBES_RING_BUFFER_SIZE_KB = 8 * 1024;
  static constexpr uint64_t MMAP_TASK_RING_BUFFER_SIZE_KB = 64;
  static constexpr uint64_t SAMPLING_RING_BUFFER_SIZE_KB = 16 * 1024;
  static constexpr uint64_t GPU_TRACING_RING_BUFFER_SIZE_KB = 256;

  static constexpr uint32_t IDLE_TIME_ON_EMPTY_RING_BUFFERS_US = 100;
  static constexpr uint32_t IDLE_TIME_ON_EMPTY_DEFERRED_EVENTS_US = 1000;

  bool trace_context_switches_;
  pid_t pid_;
  uint64_t sampling_period_ns_;
  CaptureOptions::UnwindingMethod unwinding_method_;
  std::vector<Function> instrumented_functions_;
  bool trace_gpu_driver_;

  TracerListener* listener_ = nullptr;

  std::vector<int> tracing_fds_;
  std::vector<PerfEventRingBuffer> ring_buffers_;

  absl::flat_hash_map<uint64_t, const Function*>
      uprobes_uretprobes_ids_to_function_;
  absl::flat_hash_set<uint64_t> uprobes_ids_;
  absl::flat_hash_set<uint64_t> uretprobes_ids_;
  absl::flat_hash_set<uint64_t> stack_sampling_ids_;
  absl::flat_hash_set<uint64_t> gpu_tracing_ids_;
  absl::flat_hash_set<uint64_t> callchain_sampling_ids_;

  std::atomic<bool> stop_deferred_thread_ = false;
  std::vector<std::unique_ptr<PerfEvent>> deferred_events_;
  std::mutex deferred_events_mutex_;
  ContextSwitchManager context_switch_manager_;
  std::shared_ptr<PerfEventProcessor2> uprobes_event_processor_;
  std::shared_ptr<GpuTracepointEventProcessor> gpu_event_processor_;

  static constexpr uint64_t THREAD_NAMES_UPDATE_DELAY_MS = 1000;
  absl::flat_hash_map<pid_t, std::string> thread_names_;
  uint64_t last_thread_names_update = 0;

  struct EventStats {
    void Reset() {
      event_count_begin_ns = MonotonicTimestampNs();
      sched_switch_count = 0;
      sample_count = 0;
      uprobes_count = 0;
      lost_count = 0;
      lost_count_per_buffer.clear();
      *unwind_error_count = 0;
      *discarded_samples_in_uretprobes_count = 0;
    }

    uint64_t event_count_begin_ns = 0;
    uint64_t sched_switch_count = 0;
    uint64_t sample_count = 0;
    uint64_t uprobes_count = 0;
    uint64_t gpu_events_count = 0;
    uint64_t lost_count = 0;
    absl::flat_hash_map<PerfEventRingBuffer*, uint64_t> lost_count_per_buffer{};
    std::shared_ptr<std::atomic<uint64_t>> unwind_error_count =
        std::make_unique<std::atomic<uint64_t>>(0);
    std::shared_ptr<std::atomic<uint64_t>>
        discarded_samples_in_uretprobes_count =
            std::make_unique<std::atomic<uint64_t>>(0);
  };

  static constexpr uint64_t EVENT_STATS_WINDOW_S = 5;
  EventStats stats_{};

  static constexpr uint64_t NS_PER_MILLISECOND = 1'000'000;
  static constexpr uint64_t NS_PER_SECOND = 1'000'000'000;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_THREAD_H_
