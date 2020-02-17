#include <OrbitLinuxTracing/Tracer.h>

#include "Logging.h"
#include "TracerThread.h"

namespace LinuxTracing {

Tracer::Tracer(pid_t pid, double sampling_frequency,
               std::vector<Function> instrumented_functions)
    : pid_{pid}, instrumented_functions_{std::move(instrumented_functions)} {
  std::optional<uint64_t> sampling_period_ns =
      ComputeSamplingPeriodNs(sampling_frequency);
  if (sampling_period_ns.has_value()) {
    sampling_period_ns_ = sampling_period_ns.value();
  } else {
    ERROR("Invalid sampling frequency %.1f, defaulting to %.1f",
          sampling_frequency, DEFAULT_SAMPLING_FREQUENCY);
    sampling_period_ns_ =
        ComputeSamplingPeriodNs(DEFAULT_SAMPLING_FREQUENCY).value();
  }
}

void Tracer::Run(pid_t pid, uint64_t sampling_period_ns,
                 const std::vector<Function>& instrumented_functions,
                 TracerListener* listener, bool trace_context_switches,
                 bool trace_callstacks, bool trace_instrumented_functions,
                 const std::shared_ptr<std::atomic<bool>>& exit_requested) {
  TracerThread session{pid, sampling_period_ns, instrumented_functions};
  session.SetListener(listener);
  session.SetTraceContextSwitches(trace_context_switches);
  session.SetTraceCallstacks(trace_callstacks);
  session.SetTraceInstrumentedFunctions(trace_instrumented_functions);
  session.Run(exit_requested);
}

}  // namespace LinuxTracing
