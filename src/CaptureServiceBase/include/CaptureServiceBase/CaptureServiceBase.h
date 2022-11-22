// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_BASE_CAPTURE_SERVICE_BASE_H_
#define CAPTURE_SERVICE_BASE_CAPTURE_SERVICE_BASE_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <stdint.h>

#include <memory>

#include "CaptureStartStopListener.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Profiling.h"
#include "OrbitBase/Result.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service_base {

// CaptureServiceBase holds common functionality that does not depend on gRPC and can be shared by
// the platform-specific native orbit capture services and the cloud collector.
class CaptureServiceBase {
 public:
  void AddCaptureStartStopListener(CaptureStartStopListener* listener);
  void RemoveCaptureStartStopListener(CaptureStartStopListener* listener);

  enum class CaptureInitializationResult { kSuccess, kAlreadyInProgress };
  enum class StopCaptureReason {
    kUnknown,
    kClientStop,
    kMemoryWatchdog,
    kExceededMaxDurationLimit,
    kGuestOrcStop,
    kGuestOrcConnectionFailure,
    kUploadFailure,
  };

  [[nodiscard]] CaptureInitializationResult InitializeCapture(
      orbit_producer_event_processor::ClientCaptureEventCollector* client_capture_event_collector);
  void TerminateCapture();

 protected:
  void StartEventProcessing(const orbit_grpc_protos::CaptureOptions& capture_options);
  void FinalizeEventProcessing(
      StopCaptureReason stop_capture_reason,
      orbit_grpc_protos::CaptureFinished::ProcessState target_process_state_after_capture =
          orbit_grpc_protos::CaptureFinished::kProcessStateUnknown,
      orbit_grpc_protos::CaptureFinished::TerminationSignal target_process_termination_signal =
          orbit_grpc_protos::CaptureFinished::kTerminationSignalUnknown);

  orbit_producer_event_processor::ClientCaptureEventCollector* client_capture_event_collector_;
  std::unique_ptr<orbit_producer_event_processor::ProducerEventProcessor> producer_event_processor_;

  absl::flat_hash_set<CaptureStartStopListener*> capture_start_stop_listeners_;
  uint64_t capture_start_timestamp_ns_ = 0;

 private:
  // We estimate clock resolution only once, not at the beginning of every capture.
  uint64_t clock_resolution_ns_ = orbit_base::EstimateAndLogClockResolution();
  absl::Mutex capture_mutex_;
  bool is_capturing_ ABSL_GUARDED_BY(capture_mutex_) = false;
};

}  // namespace orbit_capture_service_base

#endif  // CAPTURE_SERVICE_BASE_CAPTURE_SERVICE_BASE_H_