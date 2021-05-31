// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <cstdint>

#include "MachineCode.h"

namespace orbit_user_space_instrumentation {

TEST(MachineCodeTest, BuildCode) {
  MachineCode code;
  int8_t kSigned8 = 0x08;
  uint32_t kUnsigned32 = 0x42;
  int32_t kSigned32 = -1;
  uint64_t kUnsigned64 = 0x54;
  code.AppendBytes({0x48, 0xc7, 0xc3})
      .AppendImmediate8(kSigned8)
      .AppendImmediate32(kUnsigned32)
      .AppendImmediate32(kSigned32)
      .AppendImmediate64(kUnsigned64);

  EXPECT_EQ(code.GetResultAsVector(),
            std::vector<uint8_t>({0x48, 0xc7, 0xc3, 0x08, 0x42, 0x00, 0x00, 0x00, 0xff, 0xff,
                                  0xff, 0xff, 0x54, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}));
}

}  // namespace orbit_user_space_instrumentation