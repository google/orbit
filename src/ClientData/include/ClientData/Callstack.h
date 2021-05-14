// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CALLSTACK_H_
#define CLIENT_DATA_CALLSTACK_H_

#include <vector>

#include "CallstackTypes.h"

namespace orbit_client_data {

class CallStack {
 public:
  CallStack() = default;
  explicit CallStack(std::vector<uint64_t>&& addresses) : frames_{std::move(addresses)} {}

  [[nodiscard]] uint64_t GetFrame(size_t index) const { return frames_.at(index); }
  [[nodiscard]] const std::vector<uint64_t>& frames() const { return frames_; };
  [[nodiscard]] size_t GetFramesCount() const { return frames_.size(); }

 private:
  std::vector<uint64_t> frames_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CALLSTACK_H_
