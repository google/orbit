// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURER_H_
#define CAPTURE_SERVICE_CAPTURER_H_

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>

#include "CaptureStartStopListener.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"
#include "ProducerEventProcessor/ClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service {

// Capturer holds common functionality that does not depend on gRPC and can be shared by the
// platform-specific native orbit capture services and the cloud collector.
class Capturer {
 public:
  Capturer();

  void AddCaptureStartStopListener(CaptureStartStopListener* listener);
  void RemoveCaptureStartStopListener(CaptureStartStopListener* listener);

 protected:
  ErrorMessageOr<void> InitializeCapture(
      std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
          client_capture_event_collector);
  void TerminateCapture();

  void StartEventProcessing(const orbit_grpc_protos::CaptureOptions& capture_options);
  enum class StopCaptureReason { kClientStop, kMemoryWatchdog };
  void FinalizeEventProcessing(StopCaptureReason stop_capture_reason);

  std::unique_ptr<orbit_producer_event_processor::ClientCaptureEventCollector>
      client_capture_event_collector_;
  std::unique_ptr<orbit_producer_event_processor::ProducerEventProcessor> producer_event_processor_;

  absl::flat_hash_set<CaptureStartStopListener*> capture_start_stop_listeners_;
  uint64_t capture_start_timestamp_ns_ = 0;

 private:
  uint64_t clock_resolution_ns_ = 0;
  absl::Mutex capture_mutex_;
  bool is_capturing_ ABSL_GUARDED_BY(capture_mutex_) = false;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURER_H_