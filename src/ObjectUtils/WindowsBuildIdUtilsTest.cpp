// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <ObjectUtils/WindowsBuildIdUtils.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <string>

namespace orbit_object_utils {

TEST(WindowsBuildIdUtils, ComputeWindowsBuildId) {
  {
    std::array<uint8_t, 16> guid{255, 255, 255, 255, 255, 255, 255, 255,
                                 255, 255, 255, 255, 255, 255, 255, 255};
    uint32_t age = 42;

    std::string build_id = ComputeWindowsBuildId(guid, age);
    EXPECT_EQ(build_id, "ffffffffffffffffffffffffffffffff-42");
  }

  {
    std::array<uint8_t, 16> guid{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    uint32_t age = 0;

    std::string build_id = ComputeWindowsBuildId(guid, age);
    EXPECT_EQ(build_id, "00000000000000000000000000000000-0");
  }

  {
    std::array<uint8_t, 16> guid{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    uint32_t age = 0;

    std::string build_id = ComputeWindowsBuildId(guid, age);
    EXPECT_EQ(build_id, "030201000504070608090a0b0c0d0e0f-0");
  }

  {
    std::array<uint8_t, 16> guid{85,  187, 209, 238, 5,  74,  79,  231,
                                 234, 236, 207, 134, 88, 181, 143, 196};
    uint32_t age = 65;

    std::string build_id = ComputeWindowsBuildId(guid, age);
    EXPECT_EQ(build_id, "eed1bb554a05e74feaeccf8658b58fc4-65");
  }
}

}  // namespace orbit_object_utils
