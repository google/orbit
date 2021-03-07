// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MachineCode.h"

namespace orbit_user_space_instrumentation {

MachineCode& MachineCode::AppendBytes(const std::vector<uint8_t>& data) {
  data_.insert(data_.end(), data.begin(), data.end());
  return *this;
}

MachineCode& MachineCode::AppendImmediate64(uint64_t data) {
  for (int i = 0; i < 8; i++) {
    data_.push_back(data & 0xff);
    data = data >> 8;
  }
  return *this;
}

MachineCode& MachineCode::AppendImmediate32(uint32_t data) {
  for (int i = 0; i < 4; i++) {
    data_.push_back(data & 0xff);
    data = data >> 8;
  }
  return *this;
}

}  // namespace orbit_user_space_instrumentation