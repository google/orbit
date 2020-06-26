// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_LINUX_TRACING_HANDLER_H_
#define ORBIT_CORE_LINUX_TRACING_HANDLER_H_

#include <OrbitLinuxTracing/Tracer.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include "ContextSwitch.h"
#include "LinuxCallstackEvent.h"
#include "LinuxTracingBuffer.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "ScopeTimer.h"
#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "capture.pb.h"

class LinuxTracingHandler : public LinuxTracing::TracerListener {
 public:
  explicit LinuxTracingHandler(LinuxTracingBuffer* tracing_buffer)
      : tracing_buffer_{tracing_buffer} {}

  ~LinuxTracingHandler() override = default;
  LinuxTracingHandler(const LinuxTracingHandler&) = delete;
  LinuxTracingHandler& operator=(const LinuxTracingHandler&) = delete;
  LinuxTracingHandler(LinuxTracingHandler&&) = delete;
  LinuxTracingHandler& operator=(LinuxTracingHandler&&) = delete;

  void Start(CaptureOptions capture_options);
  bool IsStarted();
  void Stop();

  void OnSchedulingSlice(SchedulingSlice scheduling_slice) override;
  void OnCallstack(const LinuxTracing::Callstack& callstack) override;
  void OnFunctionCall(const LinuxTracing::FunctionCall& function_call) override;
  void OnGpuJob(const LinuxTracing::GpuJob& gpu_job) override;
  void OnThreadName(pid_t tid, const std::string& name) override;

 private:
  uint64_t ProcessStringAndGetKey(const std::string& string);

  LinuxTracingBuffer* tracing_buffer_;
  std::unique_ptr<LinuxTracing::Tracer> tracer_;

  absl::flat_hash_set<uint64_t> addresses_seen_;
  absl::Mutex addresses_seen_mutex_;
  absl::flat_hash_set<uint64_t> callstack_hashes_seen_;
  absl::Mutex callstack_hashes_seen_mutex_;
  StringManager string_manager_;
};

#endif  // ORBIT_CORE_LINUX_TRACING_HANDLER_H_
