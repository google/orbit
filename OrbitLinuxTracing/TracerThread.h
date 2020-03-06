#ifndef ORBIT_LINUX_TRACING_TRACER_THREAD_H_
#define ORBIT_LINUX_TRACING_TRACER_THREAD_H_

#include <OrbitLinuxTracing/Events.h>
#include <OrbitLinuxTracing/Function.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include <atomic>
#include <memory>
#include <optional>
#include <regex>
#include <vector>

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
  TracerThread(TracerThread&&) = default;
  TracerThread& operator=(TracerThread&&) = default;

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
  // Number of records to read consecutively from a perf_event_open ring buffer
  // before switching to another one.
  static constexpr int32_t ROUND_ROBIN_POLLING_BATCH_SIZE = 5;
  static constexpr uint64_t SMALL_RING_BUFFER_SIZE_KB = 256;
  static constexpr uint64_t BIG_RING_BUFFER_SIZE_KB = 2048;

  pid_t pid_;
  uint64_t sampling_period_ns_;
  std::vector<Function> instrumented_functions_;

  TracerListener* listener_ = nullptr;

  bool trace_context_switches_ = true;
  bool trace_callstacks_ = true;
  bool trace_instrumented_functions_ = true;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_THREAD_H_
