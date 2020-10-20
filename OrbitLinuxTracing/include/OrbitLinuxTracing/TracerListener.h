// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
#define ORBIT_LINUX_TRACING_TRACER_LISTENER_H_

#include "capture.pb.h"

namespace LinuxTracing {

class TracerListener {
 public:
  virtual ~TracerListener() = default;
  virtual void OnSchedulingSlice(orbit_grpc_protos::SchedulingSlice scheduling_slice) = 0;
  virtual void OnCallstackSample(orbit_grpc_protos::CallstackSample callstack_sample) = 0;
  virtual void OnFunctionCall(orbit_grpc_protos::FunctionCall function_call) = 0;
  virtual void OnIntrospectionCall(orbit_grpc_protos::IntrospectionCall introspection_call) = 0;
  virtual void OnGpuJob(orbit_grpc_protos::GpuJob gpu_job) = 0;
  virtual void OnThreadName(orbit_grpc_protos::ThreadName thread_name) = 0;
  virtual void OnThreadStateSlice(orbit_grpc_protos::ThreadStateSlice thread_state_slice) = 0;
  virtual void OnAddressInfo(orbit_grpc_protos::AddressInfo address_info) = 0;
  virtual void OnTracepointEvent(orbit_grpc_protos::TracepointEvent tracepoint_event) = 0;
};

}  // namespace LinuxTracing

#endif  // ORBIT_LINUX_TRACING_TRACER_LISTENER_H_
