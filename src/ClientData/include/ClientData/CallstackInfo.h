// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_INFO_H_
#define CLIENT_DATA_CALLSTACK_INFO_H_

#include <stdint.h>

#include "ClientData/CallstackType.h"
namespace orbit_client_data {

// This class is used on the client to represent a unique callstack, containing the frames (as
// program counters) as well as a `CallstackType`.
class CallstackInfo {
 public:
  CallstackInfo() = delete;
  CallstackInfo(std::vector<uint64_t> frames, CallstackType type)
      : frames_{std::move(frames)}, type_{type} {}
  CallstackInfo(const CallstackInfo&) = default;
  CallstackInfo& operator=(const CallstackInfo&) = default;
  CallstackInfo(CallstackInfo&&) = default;
  CallstackInfo& operator=(CallstackInfo&&) = default;

  [[nodiscard]] const std::vector<uint64_t>& frames() const { return frames_; }
  [[nodiscard]] CallstackType type() const { return type_; }
  void set_type(CallstackType type) { type_ = type; }

  [[nodiscard]] bool IsUnwindingError() const { return type_ != CallstackType::kComplete; }

  friend bool operator==(const CallstackInfo& lhs, const CallstackInfo& rhs) {
    return lhs.type_ == rhs.type_ && lhs.frames_ == rhs.frames_;
  }

  friend bool operator!=(const CallstackInfo& lhs, const CallstackInfo& rhs) {
    return !(lhs == rhs);
  }

  template <typename H>
  friend H AbslHashValue(H h, const CallstackInfo& o) {
    return H::combine(std::move(h), o.type_, o.frames_);
  }

 private:
  std::vector<uint64_t> frames_;
  CallstackType type_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_EVENT_H_