// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_CAPTURE_CLIENT_H_
#define CAPTURE_CLIENT_CAPTURE_CLIENT_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/channel.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <atomic>
#include <memory>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureClient/CaptureListener.h"
#include "CaptureClient/ClientCaptureOptions.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"

namespace orbit_capture_client {

class CaptureClient {
 public:
  enum class State { kStopped = 0, kStarting, kStarted, kStopping };

  explicit CaptureClient(const std::shared_ptr<grpc::Channel>& channel)
      : capture_service_{orbit_grpc_protos::CaptureService::NewStub(channel)} {}

  orbit_base::Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> Capture(
      orbit_base::ThreadPool* thread_pool,
      std::unique_ptr<CaptureEventProcessor> capture_event_processor,
      const orbit_client_data::ModuleManager& module_manager,
      const orbit_client_data::ProcessData& process_data,
      const ClientCaptureOptions& capture_options);

  // Returns true if stop was initiated and false otherwise.
  // The latter can happen if for example the stop was already
  // initiated.
  //
  // This call may block if the capture is in kStarting state,
  // it will wait until capture is started or failed to start.
  [[nodiscard]] bool StopCapture();

  [[nodiscard]] State state() const {
    absl::MutexLock lock(&state_mutex_);
    return state_;
  }

  [[nodiscard]] bool IsCapturing() const {
    absl::MutexLock lock(&state_mutex_);
    return state_ != State::kStopped;
  }

  bool AbortCaptureAndWait(int64_t max_wait_ms);

 private:
  ErrorMessageOr<CaptureListener::CaptureOutcome> CaptureSync(
      orbit_grpc_protos::CaptureOptions capture_options,
      CaptureEventProcessor* capture_event_processor);

  void ProcessEvents(
      CaptureEventProcessor* capture_event_processor,
      const google::protobuf::RepeatedPtrField<orbit_grpc_protos::ClientCaptureEvent>& events);

  [[nodiscard]] ErrorMessageOr<void> FinishCapture();

  std::unique_ptr<orbit_grpc_protos::CaptureService::Stub> capture_service_;
  std::unique_ptr<grpc::ClientContext> client_context_;
  std::unique_ptr<grpc::ClientReaderWriter<orbit_grpc_protos::CaptureRequest,
                                           orbit_grpc_protos::CaptureResponse>>
      reader_writer_;
  absl::Mutex context_and_stream_mutex_;

  mutable absl::Mutex state_mutex_;
  State state_ = State::kStopped;
  std::atomic<bool> writes_done_failed_ = false;
  std::atomic<bool> try_abort_ = false;
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_CAPTURE_CLIENT_H_
