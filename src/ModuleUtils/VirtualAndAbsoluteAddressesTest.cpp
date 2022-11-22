// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>
#include <tuple>

#include "ModuleUtils/VirtualAndAbsoluteAddresses.h"

namespace orbit_module_utils {

TEST(VirtualAndAbsoluteAddresses, SymbolVirtualAddressToAbsoluteAddress) {
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x10, 0x1000, 0, 0), 0x1010);
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x1010, 0x2000, 0x1000, 0), 0x2010);
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x100, 0x1000, 0, 0xFF), 0x1100);
  EXPECT_EQ(SymbolVirtualAddressToAbsoluteAddress(0x2100, 0x5000, 0x1000, 0x10FF), 0x5100);

  // Invalid input
  EXPECT_DEATH(std::ignore = SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5001, 0x1000, 0x10FF),
               "Check failed");
  EXPECT_DEATH(std::ignore = SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5001, 0x1001, 0x10FF),
               "Check failed");
  EXPECT_DEATH(std::ignore = SymbolVirtualAddressToAbsoluteAddress(0x1100, 0x5000, 0x1001, 0x10FF),
               "Check failed");
}

TEST(VirtualAndAbsoluteAddresses, SymbolAbsoluteAddressToVirtualAddress) {
  EXPECT_EQ(SymbolAbsoluteAddressToVirtualAddress(0x1010, 0x1000, 0, 0), 0x10);
  EXPECT_EQ(SymbolAbsoluteAddressToVirtualAddress(0x2010, 0x2000, 0x1000, 0), 0x1010);
  EXPECT_EQ(SymbolAbsoluteAddressToVirtualAddress(0x1100, 0x1000, 0, 0xFF), 0x100);
  EXPECT_EQ(SymbolAbsoluteAddressToVirtualAddress(0x5100, 0x5000, 0x1000, 0x10FF), 0x2100);

  // Invalid input
  EXPECT_DEATH(std::ignore = SymbolAbsoluteAddressToVirtualAddress(0x5100, 0x5001, 0x1000, 0x10FF),
               "Check failed");
  EXPECT_DEATH(std::ignore = SymbolAbsoluteAddressToVirtualAddress(0x5100, 0x5001, 0x1001, 0x10FF),
               "Check failed");
  EXPECT_DEATH(std::ignore = SymbolAbsoluteAddressToVirtualAddress(0x5100, 0x5000, 0x1001, 0x10FF),
               "Check failed");

  EXPECT_DEATH(std::ignore = SymbolAbsoluteAddressToVirtualAddress(0x5005, 0x5000, 0x1000, 0x1010),
               "Check failed");
}

}  // namespace orbit_module_utils
