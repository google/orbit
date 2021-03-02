// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CAPTURE_CLIENT_CAPTURE_CLIENT_H_
#define ORBIT_CAPTURE_CLIENT_CAPTURE_CLIENT_H_

#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>
#include <stdint.h>

#include <atomic>
#include <memory>

#include "CaptureListener.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitClientData/ModuleManager.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "absl/container/flat_hash_set.h"
#include "capture_data.pb.h"
#include "grpcpp/channel.h"
#include "services.grpc.pb.h"
#include "services.pb.h"

class CaptureClient {
 public:
  enum class State { kStopped = 0, kStarting, kStarted, kStopping };

  explicit CaptureClient(const std::shared_ptr<grpc::Channel>& channel,
                         CaptureListener* capture_listener)
      : capture_service_{orbit_grpc_protos::CaptureService::NewStub(channel)},
        capture_listener_{capture_listener} {
    CHECK(capture_listener_ != nullptr);
  }

  orbit_base::Future<ErrorMessageOr<CaptureListener::CaptureOutcome>> Capture(
      ThreadPool* thread_pool, const ProcessData& process,
      const orbit_client_data::ModuleManager& module_manager,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
      TracepointInfoSet selected_tracepoints,
      absl::flat_hash_set<uint64_t> frame_track_function_ids, bool collect_thread_state,
      bool bulk_capture_events, bool enable_introspection,
      uint64_t max_local_marker_depth_per_command_buffer);

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
      ProcessData&& process, const orbit_client_data::ModuleManager& module_manager,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
      TracepointInfoSet selected_tracepoints,
      absl::flat_hash_set<uint64_t> frame_track_function_ids, bool collect_thread_state,
      bool bulk_capture_events, bool enable_introspection,
      uint64_t max_local_marker_depth_per_command_buffer);

  [[nodiscard]] ErrorMessageOr<void> FinishCapture();

  std::unique_ptr<orbit_grpc_protos::CaptureService::Stub> capture_service_;
  std::unique_ptr<grpc::ClientContext> client_context_;
  std::unique_ptr<grpc::ClientReaderWriter<orbit_grpc_protos::CaptureRequest,
                                           orbit_grpc_protos::CaptureResponse>>
      reader_writer_;
  absl::Mutex context_and_stream_mutex_;

  CaptureListener* capture_listener_ = nullptr;

  mutable absl::Mutex state_mutex_;
  State state_ = State::kStopped;
  std::atomic<bool> writes_done_failed_ = false;
  std::atomic<bool> try_abort_ = false;
};

#endif  // ORBIT_GL_CAPTURE_CLIENT_H_
