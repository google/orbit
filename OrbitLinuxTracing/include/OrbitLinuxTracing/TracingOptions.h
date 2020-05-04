#ifndef ORBIT_LINUX_TRACING_TRACING_OPTIONS_H_
#define ORBIT_LINUX_TRACING_TRACING_OPTIONS_H_

namespace LinuxTracing {
enum SamplingMethod { kOff = 0, kFramePointers, kDwarf };

struct TracingOptions {
  bool trace_context_switches = true;
  SamplingMethod sampling_method = kDwarf;
  bool trace_instrumented_functions = true;
  bool trace_gpu_driver = true;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACING_OPTIONS_H_
