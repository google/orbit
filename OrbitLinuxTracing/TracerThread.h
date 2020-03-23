#ifndef ORBIT_LINUX_TRACING_TRACER_THREAD_H_
#define ORBIT_LINUX_TRACING_TRACER_THREAD_H_

#include <OrbitLinuxTracing/Events.h>
#include <OrbitLinuxTracing/Function.h>
#include <OrbitLinuxTracing/TracerListener.h>
#include <linux/perf_event.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <optional>
#include <regex>
#include <vector>

#include "PerfEvent.h"
#include "PerfEventProcessor.h"
#include "PerfEventProcessor2.h"
#include "PerfEventReaders.h"
#include "PerfEventRingBuffer.h"
#include "Utils.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

namespace LinuxTracing {

class TracerThread {
 public:
  TracerThread(pid_t pid, uint64_t sampling_period_ns,
               std::vector<Function> instrumented_functions)
      : pid_(pid),
        sampling_period_ns_(sampling_period_ns),
        instrumented_functions_(std::move(instrumented_functions)) {}

  TracerThread(const TracerThread&) = delete;
  TracerThread& operator=(const TracerThread&) = delete;
  TracerThread(TracerThread&&) = delete;
  TracerThread& operator=(TracerThread&&) = delete;

  void SetListener(TracerListener* listener) { listener_ = listener; }

  void SetTraceContextSwitches(bool trace_context_switches) {
    trace_context_switches_ = trace_context_switches;
  }

  void SetTraceCallstacks(bool trace_callstacks) {
    trace_callstacks_ = trace_callstacks;
  }

  void SetTraceInstrumentedFunctions(bool trace_instrumented_functions) {
    trace_instrumented_functions_ = trace_instrumented_functions;
  }

  void Run(const std::shared_ptr<std::atomic<bool>>& exit_requested);

 private:
  bool OpenRingBufferForGpuTracepoint(
      const char* tracepoint_category, const char* tracepoint_name, int32_t cpu,
      std::vector<PerfEventRingBuffer>* ring_buffers);

  bool OpenGpuTracepoints(const std::vector<int32_t>& cpus);
  void CleanupGpuTracepoints();

  void ProcessContextSwitchEvent(const perf_event_header& header,
                                 PerfEventRingBuffer* ring_buffer);
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

  void Reset();
  void PrintStatsIfTimerElapsed();

  void DeferEvent(std::unique_ptr<PerfEvent> event);
  std::vector<std::unique_ptr<PerfEvent>> ConsumeDeferredEvents();
  void ProcessDeferredEvents();

  // Number of records to read consecutively from a perf_event_open ring buffer
  // before switching to another one.
  static constexpr int32_t ROUND_ROBIN_POLLING_BATCH_SIZE = 5;

  static constexpr uint64_t SMALL_RING_BUFFER_SIZE_KB = 256;
  static constexpr uint64_t BIG_RING_BUFFER_SIZE_KB = 2048;

  static constexpr uint32_t IDLE_TIME_ON_EMPTY_RING_BUFFERS_US = 100;
  static constexpr uint32_t IDLE_TIME_ON_EMPTY_DEFERRED_EVENTS_US = 1000;

  pid_t pid_;
  uint64_t sampling_period_ns_;
  std::vector<Function> instrumented_functions_;

  TracerListener* listener_ = nullptr;

  bool trace_context_switches_ = true;
  bool trace_callstacks_ = true;
  bool trace_instrumented_functions_ = true;
  bool trace_gpu_driver_events_ = true;

  std::vector<int> tracing_fds_;
  std::vector<PerfEventRingBuffer> ring_buffers_;
  absl::flat_hash_map<int, const Function*> uprobes_fds_to_function_;
  absl::flat_hash_set<int> gpu_tracing_fds_;

  std::atomic<bool> stop_deferred_thread_ = false;
  std::vector<std::unique_ptr<PerfEvent>> deferred_events_;
  std::mutex deferred_events_mutex_;
  std::shared_ptr<PerfEventProcessor2> uprobes_event_processor_;

  struct EventStats {
    void Reset() { *this = EventStats(); }
    uint64_t event_count_begin_ns = MonotonicTimestampNs();
    uint64_t sched_switch_count = 0;
    uint64_t sample_count = 0;
    uint64_t uprobes_count = 0;
    uint64_t gpu_events_count = 0;
    uint64_t lost_count = 0;
    absl::flat_hash_map<PerfEventRingBuffer*, uint64_t> lost_count_per_buffer{};
  };

  EventStats stats_;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_THREAD_H_
