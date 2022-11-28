// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureClient/LoadCapture.h"

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <google/protobuf/stubs/port.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>

#include "CaptureClient/CaptureEventProcessor.h"
#include "CaptureFile/CaptureFileSection.h"
#include "CaptureFile/ProtoSectionInputStream.h"
#include "ClientProtos/user_defined_capture_info.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_capture_client {

[[nodiscard]] ErrorMessageOr<CaptureListener::CaptureOutcome> LoadCapture(
    CaptureListener* listener, orbit_capture_file::CaptureFile* capture_file,
    std::atomic<bool>* capture_loading_cancellation_requested) {
  {
    ORBIT_SCOPED_TIMED_LOG("Loading capture from \"%s\"", capture_file->GetFilePath().string());
    absl::flat_hash_set<uint64_t> frame_track_function_ids;

    std::optional<uint64_t> section_index =
        capture_file->FindSectionByType(orbit_capture_file::kSectionTypeUserData);
    if (section_index.has_value()) {
      orbit_client_protos::UserDefinedCaptureInfo user_defined_capture_info;
      auto proto_input_stream = capture_file->CreateProtoSectionInputStream(section_index.value());
      OUTCOME_TRY(proto_input_stream->ReadMessage(&user_defined_capture_info));
      const auto& loaded_frame_track_function_ids =
          user_defined_capture_info.frame_tracks_info().frame_track_function_ids();
      frame_track_function_ids = {loaded_frame_track_function_ids.begin(),
                                  loaded_frame_track_function_ids.end()};
    }

    std::unique_ptr<CaptureEventProcessor> capture_event_processor =
        CaptureEventProcessor::CreateForCaptureListener(listener, capture_file->GetFilePath(),
                                                        frame_track_function_ids);

    auto capture_section_input_stream = capture_file->CreateCaptureSectionInputStream();
    while (true) {
      if (*capture_loading_cancellation_requested) {
        return CaptureListener::CaptureOutcome::kCancelled;
      }
      orbit_grpc_protos::ClientCaptureEvent event;
      OUTCOME_TRY(capture_section_input_stream->ReadMessage(&event));
      capture_event_processor->ProcessEvent(event);
      if (event.event_case() == orbit_grpc_protos::ClientCaptureEvent::kCaptureFinished) {
        return CaptureListener::CaptureOutcome::kComplete;
      }
    }
  }
}

}  // namespace orbit_capture_client