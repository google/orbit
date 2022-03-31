// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ClientData/UserDefinedCaptureData.h"

namespace orbit_client_data {

void UserDefinedCaptureData::InsertFrameTrack(const FunctionInfo& function) {
  frame_track_functions_.insert(function);
}

void UserDefinedCaptureData::EraseFrameTrack(const FunctionInfo& function) {
  frame_track_functions_.erase(function);
}

[[nodiscard]] bool UserDefinedCaptureData::ContainsFrameTrack(const FunctionInfo& function) const {
  return frame_track_functions_.contains(function);
}

}  // namespace orbit_client_data
