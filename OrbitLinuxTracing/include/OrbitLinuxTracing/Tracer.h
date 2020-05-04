#ifndef ORBIT_LINUX_TRACING_TRACER_H_
#define ORBIT_LINUX_TRACING_TRACER_H_

#include <OrbitLinuxTracing/Events.h>
#include <OrbitLinuxTracing/Function.h>
#include <OrbitLinuxTracing/TracerListener.h>
#include <OrbitLinuxTracing/TracingOptions.h>
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
    tracing_options_.trace_context_switches = trace_context_switches;
  }

  void SetSamplingMethod(SamplingMethod sampling_method) {
    tracing_options_.sampling_method = sampling_method;
  }

  void SetTraceInstrumentedFunctions(bool trace_instrumented_functions) {
    tracing_options_.trace_instrumented_functions =
        trace_instrumented_functions;
  }

  void SetTraceGpuDriver(bool trace_gpu_driver) {
    tracing_options_.trace_gpu_driver = trace_gpu_driver;
  }

  void Start() {
    *exit_requested_ = false;
    thread_ = std::make_shared<std::thread>(
        &Tracer::Run, pid_, sampling_period_ns_, instrumented_functions_,
        listener_, tracing_options_, exit_requested_);
  }

  bool IsTracing() { return thread_ != nullptr && thread_->joinable(); }

  void Stop() {
    *exit_requested_ = true;
    if (thread_ != nullptr && thread_->joinable()) {
      thread_->join();
    }
    thread_.reset();
  }

 private:
  pid_t pid_;
  uint64_t sampling_period_ns_;
  std::vector<Function> instrumented_functions_;

  TracerListener* listener_ = nullptr;
  TracingOptions tracing_options_{.trace_context_switches = true,
                                  .sampling_method = kDwarf,
                                  .trace_instrumented_functions = true,
                                  .trace_gpu_driver = true};

  // exit_requested_ must outlive this object because it is used by thread_.
  // The control block of shared_ptr is thread safe (i.e., reference counting
  // and pointee's lifetime management are atomic and thread safe).
  std::shared_ptr<std::atomic<bool>> exit_requested_ =
      std::make_unique<std::atomic<bool>>(true);
  std::shared_ptr<std::thread> thread_;

  static void Run(pid_t pid, uint64_t sampling_period_ns,
                  const std::vector<Function>& instrumented_functions,
                  TracerListener* listener,
                  const TracingOptions& tracing_options,
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
