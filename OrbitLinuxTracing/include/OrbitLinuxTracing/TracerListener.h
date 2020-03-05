#ifndef ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
#define ORBIT_LINUX_TRACING_TRACER_LISTENER_H_

#include <OrbitLinuxTracing/Events.h>

namespace LinuxTracing {

class TracerListener {
 public:
  virtual ~TracerListener() = default;
  virtual void OnTid(pid_t tid) = 0;
  virtual void OnContextSwitchIn(const ContextSwitchIn& context_switch_in) = 0;
  virtual void OnContextSwitchOut(
      const ContextSwitchOut& context_switch_out) = 0;
  virtual void OnCallstack(const Callstack& callstack) = 0;
  virtual void OnFunctionCall(const FunctionCall& function_call) = 0;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
