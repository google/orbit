// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_
#define CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_

#include <absl/container/flat_hash_set.h>

#include <cstdint>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "CaptureClient/CaptureListener.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_client {

class CaptureEventProcessor {
 public:
  CaptureEventProcessor() = default;
  virtual ~CaptureEventProcessor() = default;

  virtual void ProcessEvent(const orbit_grpc_protos::ClientCaptureEvent& event) = 0;

  static std::unique_ptr<CaptureEventProcessor> CreateForCaptureListener(
      CaptureListener* capture_listener, std::optional<std::filesystem::path> file_path,
      absl::flat_hash_set<uint64_t> frame_track_function_ids);

  static ErrorMessageOr<std::unique_ptr<CaptureEventProcessor>> CreateSaveToFileProcessor(
      const std::filesystem::path& file_path,
      std::function<void(const ErrorMessage&)> error_handler);

  static std::unique_ptr<CaptureEventProcessor> CreateCompositeProcessor(
      std::vector<std::unique_ptr<CaptureEventProcessor>> event_processors);
};

}  // namespace orbit_capture_client

#endif  // CAPTURE_CLIENT_CAPTURE_EVENT_PROCESSOR_H_
