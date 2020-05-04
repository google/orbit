#ifndef ORBITLINUXTRACING_INCLUDE_ORBITLINUXTRACING_TRACINGOPTIONS_H_
#define ORBITLINUXTRACING_INCLUDE_ORBITLINUXTRACING_TRACINGOPTIONS_H_

namespace LinuxTracing {
enum UnwindingMethod {
  kUndefined = 0,
  kFramePointers,
  kDwarf
};

struct TracingOptions {
  bool trace_context_switches_ = true;
  bool trace_callstacks_ = true;
  UnwindingMethod unwinding_method_ = kDwarf;
  bool trace_instrumented_functions_ = true;
  bool trace_gpu_driver_ = true;
};

}  // namespace LinuxTracing

#endif //ORBITLINUXTRACING_INCLUDE_ORBITLINUXTRACING_TRACINGOPTIONS_H_
