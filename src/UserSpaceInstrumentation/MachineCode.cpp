// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MachineCode.h"

#include <absl/base/casts.h>

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

MachineCode& MachineCode::AppendImmediate32(int32_t data) {
  return AppendImmediate32(absl::bit_cast<uint32_t>(data));
}

MachineCode& MachineCode::AppendImmediate8(int8_t data) {
  data_.push_back(absl::bit_cast<uint8_t>(data));
  return *this;
}

}  // namespace orbit_user_space_instrumentation