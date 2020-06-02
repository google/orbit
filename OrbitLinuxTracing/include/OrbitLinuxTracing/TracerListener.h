// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

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
  virtual void OnGpuJob(const GpuJob& gpu_job) = 0;
  virtual void OnThreadName(pid_t tid, const std::string& name) = 0;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
