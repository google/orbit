// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/hash/hash.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <cstring>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleManager.h"
#include "OrbitGl/FormatCallstackForTooltip.h"

using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;
using orbit_client_data::CaptureData;
using orbit_client_data::LinuxAddressInfo;
using orbit_client_data::ModuleManager;

namespace orbit_gl {

namespace {
constexpr uint64_t kFrame1 = 0xADD5E55;
constexpr uint64_t kOffsetInFunction = 0x12;
constexpr const char* kModulePath = "/path/to/module";
constexpr const char* kModuleName = "module";
constexpr const char* kFunctionNameWithSpecialChars = "void foo<int>(const Foo&)";
constexpr const char* kEscapedFunctionName = "void foo&lt;int&gt;(const Foo&amp;)";

const CallstackInfo kEmptyCallstack{{}, CallstackType::kDwarfUnwindingError};
const CaptureData kEmptyCaptureData{{}, {}, {}, CaptureData::DataSource::kLiveCapture};
const ModuleManager kModuleManager{};

const CallstackInfo kOneFrameCallstack{{kFrame1}, CallstackType::kComplete};

const orbit_client_data::LinuxAddressInfo kAddressInfoForFunctionNameWithSpecialChars{
    kFrame1, kOffsetInFunction, kModulePath, kFunctionNameWithSpecialChars};
}  // namespace

TEST(FormatInnermostFrameOfCallstackForTooltip, EmptyCallstackYieldsUnkownModuleAndFunctionName) {
  FormattedModuleAndFunctionName module_and_function_name =
      FormatInnermostFrameOfCallstackForTooltip(kEmptyCallstack, kEmptyCaptureData, kModuleManager);

  EXPECT_EQ(module_and_function_name.module_name,
            absl::StrCat("<i>", orbit_client_data::kUnknownFunctionOrModuleName, "</i>"));
  EXPECT_EQ(module_and_function_name.function_name,
            absl::StrCat("<i>", orbit_client_data::kUnknownFunctionOrModuleName, "</i>"));
}

TEST(FormatInnermostFrameOfCallstackForTooltip, PerformsHtmlEscaping) {
  CaptureData capture_data{{}, {}, {}, CaptureData::DataSource::kLiveCapture};
  capture_data.InsertAddressInfo(kAddressInfoForFunctionNameWithSpecialChars);

  FormattedModuleAndFunctionName module_and_function_name =
      FormatInnermostFrameOfCallstackForTooltip(kOneFrameCallstack, capture_data, kModuleManager);

  EXPECT_EQ(module_and_function_name.module_name, kModuleName);
  EXPECT_EQ(module_and_function_name.function_name, kEscapedFunctionName);
}

TEST(FormatCallstackForTooltip, EmptyCallstackYieldsEmptyString) {
  std::string formatted_callstack =
      FormatCallstackForTooltip(kEmptyCallstack, kEmptyCaptureData, kModuleManager);

  EXPECT_EQ(formatted_callstack, "");
}

TEST(FormatCallstackForTooltip, PerformsHtmlEscaping) {
  CaptureData capture_data{{}, {}, {}, CaptureData::DataSource::kLiveCapture};
  capture_data.InsertAddressInfo(kAddressInfoForFunctionNameWithSpecialChars);

  std::string formatted_callstack =
      FormatCallstackForTooltip(kOneFrameCallstack, capture_data, kModuleManager);

  EXPECT_EQ(formatted_callstack, absl::StrCat(kModuleName, " | ", kEscapedFunctionName, "<br/>"));
}

TEST(FormatCallstackForTooltip, ShortensLongFunctionNames) {
  CaptureData capture_data{{}, {}, {}, CaptureData::DataSource::kLiveCapture};
  constexpr const char* kLongFunctionName = "void very_very_very_very_long_function_name(int,int)";
  const LinuxAddressInfo address_info{kFrame1, kOffsetInFunction, kModulePath, kLongFunctionName};
  capture_data.InsertAddressInfo(address_info);
  constexpr size_t kMaxLineLength = 24;

  std::string formatted_callstack =
      FormatCallstackForTooltip(kOneFrameCallstack, capture_data, kModuleManager, kMaxLineLength);

  constexpr const char* kShortenedFunctionName = "void v...t,int)";
  EXPECT_EQ(formatted_callstack, absl::StrCat(kModuleName, " | ", kShortenedFunctionName, "<br/>"));
  EXPECT_EQ(formatted_callstack.size(), kMaxLineLength + std::strlen("<br/>"));
}

TEST(FormatCallstackForTooltip, ShortensLongCallstacks) {
  constexpr uint64_t kFrame2to10 = 0x1ADD5E55;
  constexpr uint64_t kFrame11 = 0x2ADD5E55;
  constexpr uint64_t kFrame12 = 0x3ADD5E55;
  const CallstackInfo callstack{
      {kFrame1, kFrame2to10, kFrame2to10, kFrame2to10, kFrame2to10, kFrame2to10, kFrame2to10,
       kFrame2to10, kFrame2to10, kFrame2to10, kFrame11, kFrame12},
      CallstackType::kComplete};
  CaptureData capture_data{{}, {}, {}, CaptureData::DataSource::kLiveCapture};

  constexpr const char* kFunction1 = "void foo(int,int)";
  const LinuxAddressInfo address_info1{kFrame1, kOffsetInFunction, kModulePath, kFunction1};
  capture_data.InsertAddressInfo(address_info1);

  constexpr const char* kFunction2to10 = "void bar(int,int)";
  const LinuxAddressInfo address_info2to10{kFrame2to10, kOffsetInFunction, kModulePath,
                                           kFunction2to10};
  capture_data.InsertAddressInfo(address_info2to10);

  constexpr const char* kFunction11 = "void baz(int,int)";
  const LinuxAddressInfo address_info11{kFrame11, kOffsetInFunction, kModulePath, kFunction11};
  capture_data.InsertAddressInfo(address_info11);

  constexpr const char* kFunction12 = "void bazbaz(int,int)";
  constexpr const char* kModulePath2 = "/path/to/module2";
  const LinuxAddressInfo address_info12{kFrame12, kOffsetInFunction, kModulePath2, kFunction12};
  capture_data.InsertAddressInfo(address_info12);

  std::string formatted_callstack = FormatCallstackForTooltip(
      callstack, capture_data, kModuleManager, std::numeric_limits<size_t>::max(), 6, 2);

  constexpr const char* kModuleName2 = "module2";

  std::string expected_formatted_callstack{};
  {
    std::vector<std::string> expected_formatted_callstack_frames{
        absl::StrCat(kModuleName, " | ", kFunction1, "<br/>"),
        absl::StrCat(kModuleName, " | ", kFunction2to10, "<br/>"),
        absl::StrCat(kModuleName, " | ", kFunction2to10, "<br/>"),
        absl::StrCat(kModuleName, " | ", kFunction2to10, "<br/>"),
        absl::StrCat("<i>... shortened for readability ...</i><br/>"),
        absl::StrCat(kModuleName, " | ", kFunction11, "<br/>"),
        absl::StrCat(kModuleName2, " | ", kFunction12, "<br/>"),
    };
    expected_formatted_callstack = absl::StrJoin(expected_formatted_callstack_frames, "");
  }

  EXPECT_EQ(formatted_callstack, expected_formatted_callstack);
}

TEST(FormatCallstackForTooltip, ColorsUnwindingErrors) {
  constexpr uint64_t kFrame2 = 0x1ADD5E55;
  constexpr uint64_t kFrame3 = 0x2ADD5E55;
  constexpr uint64_t kFrame4 = 0x3ADD5E55;
  const CallstackInfo callstack{{kFrame1, kFrame2, kFrame3, kFrame4},
                                CallstackType::kDwarfUnwindingError};
  CaptureData capture_data{{}, {}, {}, CaptureData::DataSource::kLiveCapture};

  constexpr const char* kFunction1 = "void foo(int,int)";
  const LinuxAddressInfo address_info1{kFrame1, kOffsetInFunction, kModulePath, kFunction1};
  capture_data.InsertAddressInfo(address_info1);

  constexpr const char* kFunction2 = "void bar(int,int)";
  const LinuxAddressInfo address_info2{kFrame2, kOffsetInFunction, kModulePath, kFunction2};
  capture_data.InsertAddressInfo(address_info2);

  constexpr const char* kFunction3 = "void baz(int,int)";
  const LinuxAddressInfo address_info3{kFrame3, kOffsetInFunction, kModulePath, kFunction3};
  capture_data.InsertAddressInfo(address_info3);

  constexpr const char* kFunction4 = "void bazbaz(int,int)";
  constexpr const char* kModulePath2 = "/path/to/module2";
  constexpr const char* kModuleName2 = "module2";
  const LinuxAddressInfo address_info4{kFrame4, kOffsetInFunction, kModulePath2, kFunction4};
  capture_data.InsertAddressInfo(address_info4);

  std::string formatted_callstack =
      FormatCallstackForTooltip(callstack, capture_data, kModuleManager);

  std::vector<std::string> expected_formatted_callstack{};
  expected_formatted_callstack.push_back(absl::StrCat(kModuleName, " | ", kFunction1, "<br/>"));
  expected_formatted_callstack.push_back(
      absl::StrCat(absl::StrFormat("<span style=\"color:%s;\">", kUnwindErrorColorString),
                   kModuleName, " | ", kFunction2, "</span><br/>"));
  expected_formatted_callstack.push_back(
      absl::StrCat(absl::StrFormat("<span style=\"color:%s;\">", kUnwindErrorColorString),
                   kModuleName, " | ", kFunction3, "</span><br/>"));
  expected_formatted_callstack.push_back(
      absl::StrCat(absl::StrFormat("<span style=\"color:%s;\">", kUnwindErrorColorString),
                   kModuleName2, " | ", kFunction4, "</span><br/>"));
  EXPECT_EQ(formatted_callstack, absl::StrJoin(expected_formatted_callstack, ""));
}

}  // namespace orbit_gl
