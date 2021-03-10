// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <cstdint>

#include "MachineCode.h"

namespace orbit_user_space_instrumentation {

TEST(MachineCodeTest, BuildCode) {
  MachineCode code;
  code.AppendBytes({0x48, 0xc7, 0xc3}).AppendImmediate32(0x42).AppendImmediate64(0x54);

  EXPECT_EQ(code.GetResultAsVector(),
            std::vector<uint8_t>({0x48, 0xc7, 0xc3, 0x42, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00}));
}

}  // namespace orbit_user_space_instrumentation