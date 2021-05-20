// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QPushButton>
#include <QTest>
#include <QTimer>
#include <filesystem>

#include "AnnotatingSourceCodeDialog.h"
#include "CodeReport/AnnotateDisassembly.h"
#include "CodeReport/AnnotatingLine.h"
#include "CodeReport/DisassemblyReport.h"
#include "CodeViewer/OwningDialog.h"
#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingManager.h"
#include "SyntaxHighlighter/X86Assembly.h"
#include "capture_data.pb.h"

using namespace std::string_view_literals;
const std::string_view kMainFunctionInstructions =
    "\x50\xbf\x04\x20\x40\x00\xe8\xe5\xfe\xff\xff\x31\xc0\x59\xc3\x90"sv;

constexpr const char* kOrgName = "The Orbit Authors";

TEST(AnnotatingSourceCodeDialog, SmokeTest) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("AnnotatingSourceCodeDialog.SmokeTest");

  const std::filesystem::path file_path =
      orbit_base::GetExecutableDir() / "testdata" / "line_info_test_binary";

  {
    orbit_source_paths_mapping::MappingManager manager{};
    manager.SetMappings({});
    manager.AppendMapping(orbit_source_paths_mapping::Mapping{
        std::filesystem::path{"."} / "..", orbit_base::GetExecutableDir() / "testdata"});
  }

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> program =
      orbit_object_utils::CreateElfFile(file_path);
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
  std::string assembly = disassembler.GetResult();
  orbit_code_report::DisassemblyReport report{std::move(disassembler), kAddressOfMainFunction};

  orbit_qt::AnnotatingSourceCodeDialog dialog{};
  auto syntax_highlighter = std::make_unique<orbit_syntax_highlighter::X86Assembly>();
  dialog.SetMainContent(QString::fromStdString(assembly), std::move(syntax_highlighter));
  dialog.SetDisassemblyCodeReport(std::move(report));

  bool callback_called = false;
  dialog.AddAnnotatingSourceCode(function_info, [&](const std::string&, const std::string&) {
    callback_called = true;
    return orbit_base::Future<ErrorMessageOr<std::filesystem::path>>{file_path};
  });

  bool source_code_loaded = false;
  QObject::connect(&dialog, &orbit_qt::AnnotatingSourceCodeDialog::SourceCodeLoaded, &dialog,
                   [&]() {
                     source_code_loaded = true;
                     dialog.close();
                   });

  QObject::connect(
      &dialog, &orbit_qt::AnnotatingSourceCodeDialog::SourceCodeAvailable, &dialog, [&]() {
        QPushButton* button = dialog.findChild<QPushButton*>("notification_action_button");

        if (button == nullptr) return;

        // If available we will click on the "Load" button to load the source.
        QTest::mouseClick(button, Qt::LeftButton);
      });

  QTimer::singleShot(std::chrono::seconds{2}, &dialog, [&]() { dialog.close(); });

  dialog.exec();

  EXPECT_TRUE(callback_called);
  EXPECT_TRUE(source_code_loaded);
}
