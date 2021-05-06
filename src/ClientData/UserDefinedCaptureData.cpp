// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/UserDefinedCaptureData.h"

#include <absl/container/flat_hash_map.h>

namespace orbit_client_data {

void UserDefinedCaptureData::InsertFrameTrack(const orbit_client_protos::FunctionInfo& function) {
  frame_track_functions_.insert(function);
}

void UserDefinedCaptureData::EraseFrameTrack(const orbit_client_protos::FunctionInfo& function) {
  frame_track_functions_.erase(function);
}

[[nodiscard]] bool UserDefinedCaptureData::ContainsFrameTrack(
    const orbit_client_protos::FunctionInfo& function) const {
  return frame_track_functions_.contains(function);
}

}  // namespace orbit_client_data
