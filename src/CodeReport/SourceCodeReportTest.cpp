// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ClientData/PostProcessedSamplingData.h"
#include "ClientProtos/capture_data.pb.h"
#include "CodeReport/SourceCodeReport.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ObjectFile.h"

using orbit_object_utils::DebugSymbols;

namespace {
class MockElfFile : public orbit_object_utils::ElfFile {
 public:
  MOCK_METHOD(ErrorMessageOr<DebugSymbols>, LoadSymbolsFromDynsym, (), (override));
  MOCK_METHOD(uint64_t, GetLoadBias, (), (const, override));
  MOCK_METHOD(uint64_t, GetExecutableSegmentOffset, (), (const, override));

  MOCK_METHOD(bool, HasDynsym, (), (const, override));
  MOCK_METHOD(bool, HasDebugInfo, (), (const, override));
  MOCK_METHOD(bool, HasGnuDebuglink, (), (const, override));
  MOCK_METHOD(bool, Is64Bit, (), (const, override));
  MOCK_METHOD(std::string, GetSoname, (), (const, override));
  MOCK_METHOD(std::string, GetBuildId, (), (const, override));
  MOCK_METHOD(ErrorMessageOr<orbit_grpc_protos::LineInfo>, GetLineInfo, (uint64_t), (override));
  MOCK_METHOD(ErrorMessageOr<orbit_grpc_protos::LineInfo>, GetDeclarationLocationOfFunction,
              (uint64_t), (override));
  MOCK_METHOD(std::optional<orbit_object_utils::GnuDebugLinkInfo>, GetGnuDebugLinkInfo, (),
              (const, override));
  MOCK_METHOD(ErrorMessageOr<orbit_grpc_protos::LineInfo>, GetLocationOfFunction, (uint64_t),
              (override));

  MOCK_METHOD(ErrorMessageOr<DebugSymbols>, LoadRawDebugSymbols, (), (override));
  MOCK_METHOD(bool, HasDebugSymbols, (), (const, override));
  MOCK_METHOD(std::string, GetName, (), (const, override));
  MOCK_METHOD(const std::filesystem::path&, GetFilePath, (), (const, override));
  MOCK_METHOD(bool, IsElf, (), (const, override));
  MOCK_METHOD(bool, IsCoff, (), (const, override));
};
}  // namespace

namespace orbit_code_report {
TEST(SourceCodeReport, Empty) {
  orbit_client_protos::FunctionInfo function_info{};
  function_info.set_pretty_name("main()");
  function_info.set_address(0x42);
  function_info.set_size(0x100);

  MockElfFile elf_file{};
  EXPECT_CALL(elf_file, GetLineInfo).Times(0);

  orbit_client_data::ThreadSampleData sample_data{};

  SourceCodeReport report{"", function_info, 0x8000, &elf_file, sample_data, 0ul};

  EXPECT_EQ(report.GetNumSamples(), 0);
  EXPECT_EQ(report.GetNumSamplesInFunction(), 0);
  EXPECT_FALSE(report.GetNumSamplesAtLine(0).has_value());
}

TEST(SourceCodeReport, Simple) {
  orbit_client_protos::FunctionInfo function_info{};
  function_info.set_pretty_name("main()");
  function_info.set_address(0x42);
  function_info.set_size(0x100);

  MockElfFile elf_file{};
  orbit_grpc_protos::LineInfo static_line_info{};
  static_line_info.set_source_file("main.cpp");
  static_line_info.set_source_line(55);
  EXPECT_CALL(elf_file, GetLineInfo)
      .Times(static_cast<int>(function_info.size()))
      .WillRepeatedly(testing::Return(static_line_info));

  constexpr size_t kAbsoluteAddress = 0x8000;

  orbit_client_data::ThreadSampleData sample_data{};
  for (size_t address = kAbsoluteAddress; address < kAbsoluteAddress + function_info.size();
       ++address) {
    sample_data.sampled_address_to_count[address] = 1;
  }

  SourceCodeReport report{"main.cpp", function_info, kAbsoluteAddress,
                          &elf_file,  sample_data,   0x6666};

  EXPECT_EQ(report.GetNumSamples(), 0x6666);
  EXPECT_EQ(report.GetNumSamplesInFunction(),
            static_cast<uint32_t>(function_info.size()));  // We simulate one sample per address
  for (int line_number = 0; line_number < 55; ++line_number) {
    EXPECT_FALSE(report.GetNumSamplesAtLine(line_number).has_value());
  }
  ASSERT_TRUE(report.GetNumSamplesAtLine(55).has_value());
  EXPECT_EQ(report.GetNumSamplesAtLine(55).value(), function_info.size());
}

TEST(SourceCodeReport, NonMatchingSourceFileName) {
  // The test should discard all line info records that refer to a different source file than the
  // one given by the FunctionInfo object.

  orbit_client_protos::FunctionInfo function_info{};
  function_info.set_pretty_name("main()");
  function_info.set_address(0x42);
  function_info.set_size(1);

  MockElfFile elf_file{};
  orbit_grpc_protos::LineInfo static_line_info{};
  static_line_info.set_source_file("main.cpp");
  static_line_info.set_source_line(55);
  EXPECT_CALL(elf_file, GetLineInfo)
      .Times(static_cast<int>(function_info.size()))
      .WillRepeatedly(testing::Return(static_line_info));

  constexpr size_t kAbsoluteAddress = 0x8000;

  orbit_client_data::ThreadSampleData sample_data{};
  for (size_t address = kAbsoluteAddress; address < kAbsoluteAddress + function_info.size();
       ++address) {
    sample_data.sampled_address_to_count[address] = 1;
  }

  SourceCodeReport report{"other.cpp", function_info, kAbsoluteAddress,
                          &elf_file,   sample_data,   0x6666};

  EXPECT_EQ(report.GetNumSamples(), 0x6666);

  // In the end we should have no samples at all because the filename didn't match
  EXPECT_EQ(report.GetNumSamplesInFunction(), 0u);

  for (int line_number = 0; line_number < 60; ++line_number) {
    EXPECT_FALSE(report.GetNumSamplesAtLine(line_number).has_value());
  }
}
}  // namespace orbit_code_report
