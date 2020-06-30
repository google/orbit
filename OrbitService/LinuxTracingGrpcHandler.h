// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SERVICE_LINUX_TRACING_GRPC_HANDLER_H_
#define ORBIT_SERVICE_LINUX_TRACING_GRPC_HANDLER_H_

#include <OrbitBase/Logging.h>
#include <OrbitLinuxTracing/Tracer.h>
#include <OrbitLinuxTracing/TracerListener.h>

#include "absl/container/flat_hash_set.h"
#include "absl/synchronization/mutex.h"
#include "services.grpc.pb.h"

class LinuxTracingGrpcHandler : public LinuxTracing::TracerListener {
 public:
  explicit LinuxTracingGrpcHandler(
      grpc::ServerReaderWriter<CaptureEvent, CaptureRequest>* reader_writer)
      : reader_writer_{reader_writer} {}

  ~LinuxTracingGrpcHandler() override = default;
  LinuxTracingGrpcHandler(const LinuxTracingGrpcHandler&) = delete;
  LinuxTracingGrpcHandler& operator=(const LinuxTracingGrpcHandler&) = delete;
  LinuxTracingGrpcHandler(LinuxTracingGrpcHandler&&) = delete;
  LinuxTracingGrpcHandler& operator=(LinuxTracingGrpcHandler&&) = delete;

  void Start(CaptureOptions capture_options);
  void Stop();

  void OnSchedulingSlice(SchedulingSlice scheduling_slice) override;
  void OnCallstackSample(CallstackSample callstack_sample) override;
  void OnFunctionCall(FunctionCall function_call) override;
  void OnGpuJob(GpuJob gpu_job) override;
  void OnThreadName(ThreadName thread_name) override;
  void OnAddressInfo(AddressInfo address_info) override;

 private:
  grpc::ServerReaderWriter<CaptureEvent, CaptureRequest>* reader_writer_;
  std::unique_ptr<LinuxTracing::Tracer> tracer_;

  static uint64_t ComputeCallstackKey(const Callstack& callstack);
  uint64_t InternCallstackIfNecessaryAndGetKey(Callstack callstack);
  static uint64_t ComputeStringKey(const std::string& str);
  uint64_t InternStringIfNecessaryAndGetKey(std::string str);

  absl::flat_hash_set<uint64_t> addresses_seen_;
  absl::Mutex addresses_seen_mutex_;
  absl::flat_hash_set<uint64_t> callstack_keys_sent_;
  absl::Mutex callstack_keys_sent_mutex_;
  absl::flat_hash_set<uint64_t> string_keys_sent_;
  absl::Mutex string_keys_sent_mutex_;

  void SenderThread();
  void SendBufferedEvents();

  std::vector<CaptureEvent> event_buffer_;
  absl::Mutex event_buffer_mutex_;
  std::thread sender_thread_;
};

#endif  // ORBIT_SERVICE_LINUX_TRACING_GRPC_HANDLER_H_
