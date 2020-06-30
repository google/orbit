// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_CAPTURE_CLIENT_H_
#define ORBIT_GL_CAPTURE_CLIENT_H_

#include "CaptureListener.h"
#include "OrbitBase/Logging.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "grpcpp/channel.h"
#include "services.grpc.pb.h"

class CaptureClient {
 public:
  explicit CaptureClient(const std::shared_ptr<grpc::Channel>& channel,
                         CaptureListener* capture_listener)
      : capture_service_{CaptureService::NewStub(channel)},
        capture_listener_{capture_listener} {
    CHECK(capture_listener_ != nullptr);
  }

  void Capture();
  void StopCapture();

 private:
  void FinishCapture();

  std::unique_ptr<CaptureService::Stub> capture_service_;
  std::unique_ptr<grpc::ClientReaderWriter<CaptureRequest, CaptureEvent>>
      reader_writer_;

  void ProcessSchedulingSlice(const SchedulingSlice& scheduling_slice);
  void ProcessInternedCallstack(InternedCallstack interned_callstack);
  void ProcessCallstackSample(const CallstackSample& callstack_sample);
  void ProcessFunctionCall(const FunctionCall& function_call);
  void ProcessInternedString(InternedString interned_string);
  void ProcessGpuJob(const GpuJob& gpu_job);
  void ProcessThreadName(const ThreadName& thread_name);
  void ProcessAddressInfo(const AddressInfo& address_info);

  absl::flat_hash_map<uint64_t, Callstack> callstack_intern_pool;
  absl::flat_hash_map<uint64_t, std::string> string_intern_pool;
  CaptureListener* capture_listener_;

  absl::flat_hash_set<uint64_t> callstack_hashes_seen_;
  uint64_t GetCallstackHashAndSendToListenerIfNecessary(
      const Callstack& callstack);
  absl::flat_hash_set<uint64_t> string_hashes_seen_;
  uint64_t GetStringHashAndSendToListenerIfNecessary(const std::string& str);
};

#endif  // ORBIT_GL_CAPTURE_CLIENT_H_
