// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_replace.h>
#include <gtest/gtest.h>

#include <filesystem>

#include "CodeReport/AnnotateDisassembly.h"
#include "CodeReport/AnnotatingLine.h"
#include "CodeReport/DisassemblyReport.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "capture_data.pb.h"

using namespace std::string_view_literals;
const std::string_view kMainFunctionInstructions =
    "\x50\xbf\x04\x20\x40\x00\xe8\xe5\xfe\xff\xff\x31\xc0\x59\xc3\x90"sv;

static void TestSimple(bool windows_line_endings) {
  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "line_info_test_binary";

  auto program = orbit_object_utils::CreateElfFile(file_path);
  ASSERT_TRUE(program.has_value()) << program.error().message();

  constexpr uint64_t kAddressOfMainFunction = 0x401140;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> decl_line_info =
      program.value()->GetDeclarationLocationOfFunction(kAddressOfMainFunction);
  ASSERT_TRUE(decl_line_info.has_value()) << decl_line_info.error().message();

  const std::filesystem::path source_file_path =
      orbit_base::GetExecutableDir() / "testdata" / "LineInfoTestBinary.cpp";

  ASSERT_TRUE(std::filesystem::exists(source_file_path));

  ErrorMessageOr<std::string> source_file_contents_or_error =
      orbit_base::ReadFileToString(source_file_path);
  ASSERT_TRUE(source_file_contents_or_error.has_value())
      << source_file_contents_or_error.error().message();
  std::string source_file_contents = std::move(source_file_contents_or_error.value());

  if (windows_line_endings) {
    // This step is only relevant on Windows where Git might check out files with Windows line
    // endings depending on the configuration.
    absl::StrReplaceAll({{"\r\n", "\n"}}, &source_file_contents);

    // Ensure that source_file_contents has Windows line endings on both platforms.
    absl::StrReplaceAll({{"\n", "\r\n"}}, &source_file_contents);
  }

  orbit_client_protos::FunctionInfo function_info{};
  function_info.set_name("main");
  function_info.set_pretty_name("main");
  function_info.set_module_path("line_info_test_binary");
  function_info.set_address(0x401140);
  function_info.set_size(kMainFunctionInstructions.size());
  function_info.set_orbit_type(
      orbit_client_protos::FunctionInfo_OrbitType::FunctionInfo_OrbitType_kNone);

  orbit_code_report::Disassembler disassembler{};
  disassembler.Disassemble(static_cast<const void*>(kMainFunctionInstructions.data()),
                           kMainFunctionInstructions.size(), 0x401140, true);
  orbit_code_report::DisassemblyReport report{std::move(disassembler), kAddressOfMainFunction};

  std::vector<orbit_code_report::AnnotatingLine> annotating_lines =
      orbit_code_report::AnnotateDisassemblyWithSourceCode(function_info, decl_line_info.value(),
                                                           source_file_contents,
                                                           program.value().get(), report);

  ASSERT_EQ(annotating_lines.size(), 3);
  auto& first_line = annotating_lines[0];
  EXPECT_EQ(first_line.line_contents, "int main() {");
  EXPECT_EQ(first_line.reference_line, 2);
  EXPECT_EQ(first_line.line_number, 12);

  auto& second_line = annotating_lines[1];
  EXPECT_EQ(second_line.line_contents, "  PrintHelloWorld();");
  EXPECT_EQ(second_line.reference_line, 3);
  EXPECT_EQ(second_line.line_number, 13);

  auto& third_line = annotating_lines[2];
  EXPECT_EQ(third_line.line_contents, "  return 0;");
  EXPECT_EQ(third_line.reference_line, 5);
  EXPECT_EQ(third_line.line_number, 14);
}

TEST(AnnotateDisassembly, Simple) { TestSimple(false); }
TEST(AnnotateDisassembly, SimpleWindowsLineEndings) { TestSimple(true); }