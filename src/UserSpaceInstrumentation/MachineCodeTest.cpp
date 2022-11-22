// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "MachineCode.h"

namespace orbit_user_space_instrumentation {

TEST(MachineCodeTest, BuildCode) {
  MachineCode code;
  constexpr int8_t kInt8 = 0x08;
  constexpr uint32_t kUInt32 = 0x32;
  constexpr int32_t kInt32 = -1;
  constexpr uint64_t kUInt64 = 0x64;
  code.AppendBytes({0x48, 0xc7, 0xc3})
      .AppendImmediate8(kInt8)
      .AppendImmediate32(kUInt32)
      .AppendImmediate32(kInt32)
      .AppendImmediate64(kUInt64);

  EXPECT_EQ(code.GetResultAsVector(),
            std::vector<uint8_t>({0x48, 0xc7, 0xc3, 0x08, 0x32, 0x00, 0x00, 0x00, 0xff, 0xff,
                                  0xff, 0xff, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
}

}  // namespace orbit_user_space_instrumentation