// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_CAPTURE_SERVICE_CAPTURE_SERVICE_H_
#define LINUX_CAPTURE_SERVICE_CAPTURE_SERVICE_H_

#include <absl/container/flat_hash_set.h>
#include <grpcpp/grpcpp.h>

#include <atomic>
#include <memory>

#include "CaptureService/CaptureService.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Profiling.h"
#include "UserSpaceInstrumentation/InstrumentProcess.h"
#include "services.grpc.pb.h"
#include "services.pb.h"

namespace orbit_linux_capture_service {

class LinuxCaptureService final : public orbit_capture_service::CaptureService {
 public:
  LinuxCaptureService() {
    instrumentation_manager_ = orbit_user_space_instrumentation::InstrumentationManager::Create();
  }

  grpc::Status Capture(
      grpc::ServerContext* context,
      grpc::ServerReaderWriter<orbit_grpc_protos::CaptureResponse,
                               orbit_grpc_protos::CaptureRequest>* reader_writer) override;

 private:
  std::unique_ptr<orbit_user_space_instrumentation::InstrumentationManager>
      instrumentation_manager_;
};

}  // namespace orbit_linux_capture_service

#endif  // LINUX_CAPTURE_SERVICE_CAPTURE_SERVICE_IMPL_H_
