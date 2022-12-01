// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <cstdint>
#include <initializer_list>
#include <optional>
#include <string_view>
#include <vector>

#include "AssemblyTestLiterals.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
#include "CodeReport/Disassembler.h"
#include "CodeReport/DisassemblyReport.h"

using orbit_code_report::kFibonacciAbsoluteAddress;
using orbit_code_report::kFibonacciAssembly;

namespace orbit_code_report {

TEST(DisassemblyReport, Empty) {
  // An empty DisassemblyReport means no capture had been taken and no sampling data is available.
  // We expect all counters to be 0 and not to crash when poking.

  orbit_code_report::Disassembler disassembler{};
  orbit_client_data::ProcessData process;
  orbit_client_data::ModuleManager module_manager;
  disassembler.Disassemble(process, module_manager,
                           static_cast<const void*>(kFibonacciAssembly.data()),
                           kFibonacciAssembly.size(), kFibonacciAbsoluteAddress, true);

  orbit_code_report::DisassemblyReport disassembly_report{disassembler, kFibonacciAbsoluteAddress};
  EXPECT_EQ(disassembly_report.GetAbsoluteFunctionAddress(), kFibonacciAbsoluteAddress);
  EXPECT_EQ(disassembly_report.GetNumSamples(), 0);
  EXPECT_EQ(disassembly_report.GetNumSamplesInFunction(), 0);

  // GetNumSamplesAtLine returns nullopt for all lines which do not have any assembly instruction
  // associated with it.
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(-1).has_value());
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(0).has_value());
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(1).has_value());
  for (int line_number = 2; line_number < 28; ++line_number) {
    ASSERT_TRUE(disassembly_report.GetNumSamplesAtLine(line_number).has_value());
    EXPECT_EQ(disassembly_report.GetNumSamplesAtLine(line_number).value(), 0);
  }
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(28).has_value());
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(29).has_value());
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(30).has_value());

  for (uint64_t address : {0x00, 0x01, 0xFF}) {
    EXPECT_EQ(disassembly_report.GetLineAtAddress(address), disassembler.GetLineAtAddress(address));
  }
  for (uint64_t address = kFibonacciAbsoluteAddress; address < kFibonacciAbsoluteAddress + 0x100;
       ++address) {
    EXPECT_EQ(disassembly_report.GetLineAtAddress(address), disassembler.GetLineAtAddress(address));
  }
}

TEST(DisassemblyReport, Simple) {
  // This creates a simple DisassemblyReport with samples in three different locations.

  orbit_code_report::Disassembler disassembler{};
  orbit_client_data::ProcessData process;
  orbit_client_data::ModuleManager module_manager;
  disassembler.Disassemble(process, module_manager,
                           static_cast<const void*>(kFibonacciAssembly.data()),
                           kFibonacciAssembly.size(), kFibonacciAbsoluteAddress, true);

  constexpr size_t kFunctionCount = 7;
  constexpr size_t kTotalCount = 42;

  orbit_client_data::ThreadSampleData thread_sample_data{};
  thread_sample_data.samples_count = kTotalCount;

  // Heads up line numbers mentioned here are 1-indexed! The line number comments in
  // `AssemblyTestLiterals.h` are 0-indexed! That's why they deviate by 1.
  thread_sample_data.sampled_address_to_count[kFibonacciAbsoluteAddress] =
      1;  // Address 0x401020 Line 2
  thread_sample_data.sampled_address_to_count[kFibonacciAbsoluteAddress + 0x10] =
      2;  // Address 0x401030 Line 6
  thread_sample_data.sampled_address_to_count[kFibonacciAbsoluteAddress + 0x2f] =
      4;  // Address 0x40104f Line 20

  orbit_code_report::DisassemblyReport disassembly_report{
      disassembler, kFibonacciAbsoluteAddress, thread_sample_data, kFunctionCount, kTotalCount};
  EXPECT_EQ(disassembly_report.GetAbsoluteFunctionAddress(), kFibonacciAbsoluteAddress);
  EXPECT_EQ(disassembly_report.GetNumSamples(), kTotalCount);
  EXPECT_EQ(disassembly_report.GetNumSamplesInFunction(), kFunctionCount);

  // GetNumSamplesAtLine returns nullopt for all lines which do not have any assembly instruction
  // associated with it.
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(-1).has_value());
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(0).has_value());
  EXPECT_FALSE(disassembly_report.GetNumSamplesAtLine(1).has_value());

  // Address: kFibonacciAbsoluteAddress
  ASSERT_TRUE(disassembly_report.GetNumSamplesAtLine(2).has_value());
  EXPECT_EQ(disassembly_report.GetNumSamplesAtLine(2).value(), 1);

  // Address: kFibonacciAbsoluteAddress + 0x10
  ASSERT_TRUE(disassembly_report.GetNumSamplesAtLine(6).has_value());
  EXPECT_EQ(disassembly_report.GetNumSamplesAtLine(6).value(), 2);

  // Address: kFibonacciAbsoluteAddress + 0x2f
  ASSERT_TRUE(disassembly_report.GetNumSamplesAtLine(20).has_value());
  EXPECT_EQ(disassembly_report.GetNumSamplesAtLine(20).value(), 4);

  // All the other lines between 2 and 28 (excluded) have assembly instruction associated, but don't
  // have any samples!
  for (int line_number = 2; line_number < 28; ++line_number) {
    if (line_number == 2 || line_number == 6 || line_number == 20) continue;
    ASSERT_TRUE(disassembly_report.GetNumSamplesAtLine(line_number).has_value()) << line_number;
    EXPECT_EQ(disassembly_report.GetNumSamplesAtLine(line_number).value(), 0);
  }
}
}  // namespace orbit_code_report