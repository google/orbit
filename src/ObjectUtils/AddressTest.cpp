// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <vector>

#include "ObjectUtils/Address.h"

namespace orbit_object_utils {

TEST(Address, SymbolVirtualAddressToAbsoluteAddress) {
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x10, 0x1000, 0, 0), 0x1010);
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x1010, 0x2000, 0x1000, 0), 0x2010);
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x100, 0x1000, 0, 0xFF), 0x1100);
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5000, 0x1000, 0x10FF), 0x4100);

  // Invalid input
  EXPECT_DEATH((void)SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5001, 0x1000, 0x10FF),
               "Check failed");
  EXPECT_DEATH((void)SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5001, 0x1001, 0x10FF),
               "Check failed");
  EXPECT_DEATH((void)SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5000, 0x1001, 0x10FF),
               "Check failed");
}

TEST(Address, SymbolOffsetToAbsoluteAddress) {
  EXPECT_EQ(SymbolOffsetToAbsoluteAddress(0x10, 0x1000, 0), 0x1010);
  EXPECT_EQ(SymbolOffsetToAbsoluteAddress(0x1010, 0x2000, 0), 0x3010);
  EXPECT_EQ(SymbolOffsetToAbsoluteAddress(0x100, 0x1000, 0xFF), 0x1100);
  EXPECT_EQ(SymbolOffsetToAbsoluteAddress(0x1100, 0x5000, 0x10FF), 0x5100);

  // Invalid input
  EXPECT_DEATH((void)SymbolOffsetToAbsoluteAddress(0x1100, 0x5001, 0x10FF), "Check failed");
}

TEST(Address, SymbolAbsoluteAddressToOffset) {
  EXPECT_EQ(SymbolAbsoluteAddressToOffset(0x10005, 0x10000, 0xE001), 0xE005);
  EXPECT_EQ(SymbolAbsoluteAddressToOffset(0x10005, 0x10000, 0xE000), 0xE005);
  EXPECT_EQ(SymbolAbsoluteAddressToOffset(0x10005, 0x10000, 0x1), 0x5);
  EXPECT_EQ(SymbolAbsoluteAddressToOffset(0x10005, 0x10000, 0x0), 0x5);

  // Invalid input
  EXPECT_DEATH((void)SymbolAbsoluteAddressToOffset(0xE005, 0x10000, 0x0), "Check failed");
  EXPECT_DEATH((void)SymbolAbsoluteAddressToOffset(0x1E005, 0x10020, 0x0), "Check failed");
  EXPECT_DEATH((void)SymbolAbsoluteAddressToOffset(0x10005, 0x10000, 0x1010), "Check failed");
}

}  // namespace orbit_object_utils
