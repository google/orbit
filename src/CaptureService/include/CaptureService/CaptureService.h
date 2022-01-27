// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_SERVICE_CAPTURE_SERVICE_H_
#define CAPTURE_SERVICE_CAPTURE_SERVICE_H_

#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>
#include <grpcpp/grpcpp.h>

#include <memory>

#include "CaptureService/CaptureServiceUtils.h"
#include "CaptureStartStopListener.h"
#include "GrpcProtos/services.grpc.pb.h"
#include "GrpcProtos/services.pb.h"
#include "OrbitBase/Result.h"
#include "ProducerEventProcessor/GrpcClientCaptureEventCollector.h"
#include "ProducerEventProcessor/ProducerEventProcessor.h"

namespace orbit_capture_service {

// CaptureService is an abstract class derived from the gRPC capture service. It holds common
// functionality shared by the platform-specific capture services.
class CaptureService : public orbit_grpc_protos::CaptureService::Service {
 public:
  CaptureService();

  void AddCaptureStartStopListener(CaptureStartStopListener* listener) {
    meta_data_.AddCaptureStartStopListener(listener);
  }
  void RemoveCaptureStartStopListener(CaptureStartStopListener* listener) {
    meta_data_.RemoveCaptureStartStopListener(listener);
  }

 protected:
  grpc::Status InitializeCapture(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);
  void TerminateCapture();

  static orbit_grpc_protos::CaptureRequest WaitForStartCaptureRequestFromClient(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);

  static void WaitForStopCaptureRequestFromClient(
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer);

  CaptureServiceMetaData meta_data_;

 private:
  absl::Mutex capture_mutex_;
  bool is_capturing_ ABSL_GUARDED_BY(capture_mutex_) = false;
};

}  // namespace orbit_capture_service

#endif  // CAPTURE_SERVICE_CAPTURE_SERVICE_H_
