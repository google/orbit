#ifndef ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
#define ORBIT_LINUX_TRACING_TRACER_LISTENER_H_

#include <OrbitLinuxTracing/Events.h>

namespace LinuxTracing {

class TracerListener {
 public:
  virtual void OnTid(pid_t tid) = 0;
  virtual void OnContextSwitchIn(const ContextSwitchIn& context_switch_in) = 0;
  virtual void OnContextSwitchOut(
      const ContextSwitchOut& context_switch_out) = 0;
  virtual void OnCallstack(const Callstack& callstack) = 0;
  virtual void OnFunctionBegin(const FunctionBegin& function_begin) = 0;
  virtual void OnFunctionEnd(const FunctionEnd& function_end) = 0;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
