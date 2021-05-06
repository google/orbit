// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_USER_DEFINED_CAPTURE_DATA_H_
#define ORBIT_CORE_USER_DEFINED_CAPTURE_DATA_H_

#include "ClientData/FunctionInfoSet.h"
#include "capture_data.pb.h"

// UserDefinedCaptureData holds any capture related data that was added by the user. Examples
// for this include frame tracks, iterators, timeline annotations or comments.
// Note that this class is not thread-safe.
class UserDefinedCaptureData {
 public:
  [[nodiscard]] const FunctionInfoSet& frame_track_functions() const {
    return frame_track_functions_;
  }
  void InsertFrameTrack(const orbit_client_protos::FunctionInfo& function);
  void EraseFrameTrack(const orbit_client_protos::FunctionInfo& function);
  [[nodiscard]] bool ContainsFrameTrack(const orbit_client_protos::FunctionInfo& function) const;
  void Clear() { frame_track_functions_.clear(); }

 private:
  FunctionInfoSet frame_track_functions_;
};

#endif  // ORBIT_CORE_USER_DEFINED_CAPTURE_DATA_H_
