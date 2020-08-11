// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CALLSTACK_H_
#define ORBIT_CORE_CALLSTACK_H_

#include <xxhash.h>

#include <vector>

#include "CallstackTypes.h"

class CallStack {
 public:
  CallStack() = default;
  explicit CallStack(std::vector<uint64_t>&& addresses) {
    frames_ = std::move(addresses);
    hash_ =
        XXH64(frames_.data(), frames_.size() * sizeof(uint64_t), 0xca1157ac);
  };

  CallstackID GetHash() const { return hash_; }
  uint64_t GetFrame(size_t index) const { return frames_.at(index); }
  const std::vector<uint64_t>& GetFrames() const { return frames_; };
  size_t GetFramesCount() const { return frames_.size(); }

 private:
  CallstackID hash_ = 0;
  std::vector<uint64_t> frames_;
};
#endif  // ORBIT_CORE_CALLSTACK_H_
