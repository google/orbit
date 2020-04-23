#ifndef ORBIT_LINUX_TRACING_TRACER_H_
#define ORBIT_LINUX_TRACING_TRACER_H_

#include <OrbitLinuxTracing/Events.h>
#include <OrbitLinuxTracing/Function.h>
#include <OrbitLinuxTracing/TracerListener.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <thread>
#include <vector>

namespace LinuxTracing {

class Tracer {
 public:
  static constexpr double DEFAULT_SAMPLING_FREQUENCY = 1000.0;

  Tracer(pid_t pid, double sampling_frequency,
         std::vector<Function> instrumented_functions);

  ~Tracer() { Stop(); }

  Tracer(const Tracer&) = delete;
  Tracer& operator=(const Tracer&) = delete;
  Tracer(Tracer&&) = default;
  Tracer& operator=(Tracer&&) = default;

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

  void SetTraceGpuDriver(bool trace_gpu_driver) {
    trace_gpu_driver_ = trace_gpu_driver;
  }

  void Start() {
    *exit_requested_ = false;
    thread_ = std::make_shared<std::thread>(
        &Tracer::Run, pid_, sampling_period_ns_, instrumented_functions_,
        listener_, trace_context_switches_, trace_callstacks_,
        trace_instrumented_functions_, trace_gpu_driver_, exit_requested_);
    thread_->detach();
  }

  void Stop() { *exit_requested_ = true; }

 private:
  pid_t pid_;
  uint64_t sampling_period_ns_;
  std::vector<Function> instrumented_functions_;

  TracerListener* listener_ = nullptr;

  bool trace_context_switches_ = true;
  bool trace_callstacks_ = true;
  bool trace_instrumented_functions_ = true;
  bool trace_gpu_driver_ = true;

  // exit_requested_ must outlive this object because it is used by thread_.
  // The control block of shared_ptr is thread safe (i.e., reference counting
  // and pointee's lifetime management are atomic and thread safe).
  std::shared_ptr<std::atomic<bool>> exit_requested_ =
      std::make_unique<std::atomic<bool>>(true);
  std::shared_ptr<std::thread> thread_;

  static void Run(pid_t pid, uint64_t sampling_period_ns,
                  const std::vector<Function>& instrumented_functions,
                  TracerListener* listener, bool trace_context_switches,
                  bool trace_callstacks, bool trace_instrumented_functions,
                  bool trace_gpu_driver,
                  const std::shared_ptr<std::atomic<bool>>& exit_requested);

  static std::optional<uint64_t> ComputeSamplingPeriodNs(
      double sampling_frequency) {
    double period_ns_dbl = 1'000'000'000 / sampling_frequency;
    if (period_ns_dbl > 0 &&
        period_ns_dbl <=
            static_cast<double>(std::numeric_limits<uint64_t>::max())) {
      return std::optional<uint64_t>(period_ns_dbl);
    } else {
      return std::optional<uint64_t>();
    }
  }
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_H_
