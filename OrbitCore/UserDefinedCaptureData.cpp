// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "UserDefinedCaptureData.h"

#include "OrbitClientData/FunctionUtils.h"

void UserDefinedCaptureData::InsertFrameTrack(const orbit_client_protos::FunctionInfo& function) {
  // We do not allow insertion of frame tracks twice.
  CHECK(frame_track_functions_.insert(function).second);
}

void UserDefinedCaptureData::EraseFrameTrack(const orbit_client_protos::FunctionInfo& function) {
  // We do not allow erasing a frame track that does not exist.
  CHECK(frame_track_functions_.erase(function));
}

[[nodiscard]] bool UserDefinedCaptureData::ContainsFrameTrack(
    const orbit_client_protos::FunctionInfo& function) const {
  return frame_track_functions_.contains(function);
}
