// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_H_
#define CLIENT_DATA_CALLSTACK_H_

#include <vector>

#include "CallstackTypes.h"

class CallStack {
 public:
  CallStack() = default;
  explicit CallStack(CallstackID id, std::vector<uint64_t>&& addresses)
      : id_{id}, frames_{std::move(addresses)} {};

  [[nodiscard]] CallstackID id() const { return id_; }
  [[nodiscard]] uint64_t GetFrame(size_t index) const { return frames_.at(index); }
  [[nodiscard]] const std::vector<uint64_t>& frames() const { return frames_; };
  [[nodiscard]] size_t GetFramesCount() const { return frames_.size(); }

 private:
  CallstackID id_ = 0;
  std::vector<uint64_t> frames_;
};
#endif  // CLIENT_DATA_CALLSTACK_H_
