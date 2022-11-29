// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_USER_DEFINED_CAPTURE_DATA_H_
#define CLIENT_DATA_USER_DEFINED_CAPTURE_DATA_H_

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>

#include "ClientData/FunctionInfo.h"

namespace orbit_client_data {

// UserDefinedCaptureData holds any capture related data that was added by the user. Examples
// for this include frame tracks, iterators, timeline annotations or comments.
// Note that this class is not thread-safe.
class UserDefinedCaptureData {
 public:
  [[nodiscard]] const absl::flat_hash_set<FunctionInfo>& frame_track_functions() const {
    return frame_track_functions_;
  }
  void InsertFrameTrack(const FunctionInfo& function);
  void EraseFrameTrack(const FunctionInfo& function);
  [[nodiscard]] bool ContainsFrameTrack(const FunctionInfo& function) const;
  void Clear() { frame_track_functions_.clear(); }

 private:
  absl::flat_hash_set<FunctionInfo> frame_track_functions_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_USER_DEFINED_CAPTURE_DATA_H_
