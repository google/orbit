// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "AssemblyTestLiterals.h"
#include "CodeReport/Disassembler.h"

using orbit_code_report::kFibonacciAbsoluteAddress;
using orbit_code_report::kFibonacciAssembly;
using orbit_code_report::kFibonacciDisassembled;

TEST(Disassembler, Disassemble) {
  orbit_code_report::Disassembler disassembler{};
  disassembler.Disassemble(static_cast<const void*>(kFibonacciAssembly.data()),
                           kFibonacciAssembly.size(), kFibonacciAbsoluteAddress, true);
  EXPECT_EQ(disassembler.GetResult(), kFibonacciDisassembled);
  EXPECT_EQ(disassembler.GetAddressAtLine(0), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(4), kFibonacciAbsoluteAddress + 0x0d);
  EXPECT_EQ(disassembler.GetAddressAtLine(12), kFibonacciAbsoluteAddress + 0x1b);
  EXPECT_EQ(disassembler.GetAddressAtLine(24), kFibonacciAbsoluteAddress + 0x37);
  EXPECT_EQ(disassembler.GetAddressAtLine(27), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(28), 0);
  EXPECT_EQ(disassembler.GetAddressAtLine(29), 0);  // 29 is the first invalid line index.
  EXPECT_EQ(disassembler.GetAddressAtLine(1024), 0);

  EXPECT_FALSE(disassembler.GetLineAtAddress(0x0).has_value());
  EXPECT_FALSE(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x0c).has_value());
  EXPECT_EQ(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x0d).value_or(0), 4);
  EXPECT_EQ(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x1b).value_or(0), 12);
  EXPECT_EQ(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0x37).value_or(0), 24);
  EXPECT_FALSE(disassembler.GetLineAtAddress(kFibonacciAbsoluteAddress + 0xdf).has_value());
  EXPECT_FALSE(disassembler.GetLineAtAddress(0x0).has_value());
}