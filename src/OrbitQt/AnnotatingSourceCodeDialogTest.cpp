// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stdint.h>

#include <QCoreApplication>
#include <QObject>
#include <QPushButton>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTest>
#include <QTimer>
#include <Qt>
#include <chrono>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "CodeReport/Disassembler.h"
#include "CodeReport/DisassemblyReport.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "OrbitQt/AnnotatingSourceCodeDialog.h"
#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingManager.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SyntaxHighlighter/X86Assembly.h"
#include "Test/Path.h"

using namespace std::string_view_literals;
const std::string_view kMainFunctionInstructions =
    "\x50\xbf\x04\x20\x40\x00\xe8\xe5\xfe\xff\xff\x31\xc0\x59\xc3\x90"sv;

constexpr const char* kOrgName = "The Orbit Authors";

TEST(AnnotatingSourceCodeDialog, SmokeTest) {
  QCoreApplication::setOrganizationName(kOrgName);
  QCoreApplication::setApplicationName("AnnotatingSourceCodeDialog.SmokeTest");

  const std::filesystem::path file_path = orbit_test::GetTestdataDir() / "line_info_test_binary";

  {
    orbit_source_paths_mapping::MappingManager manager{};
    manager.SetMappings({});
    manager.AppendMapping(orbit_source_paths_mapping::Mapping{std::filesystem::path{"."} / "..",
                                                              orbit_test::GetTestdataDir()});
  }

  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> program =
      orbit_object_utils::CreateElfFile(file_path);
  ASSERT_TRUE(program.has_value()) << program.error().message();

  constexpr uint64_t kAddressOfMainFunction = 0x401140;
  ErrorMessageOr<orbit_grpc_protos::LineInfo> decl_line_info =
      program.value()->GetLocationOfFunction(kAddressOfMainFunction);
  ASSERT_TRUE(decl_line_info.has_value()) << decl_line_info.error().message();

  const std::filesystem::path source_file_path =
      orbit_test::GetTestdataDir() / "LineInfoTestBinary.cpp";

  ASSERT_TRUE(std::filesystem::exists(source_file_path));

  ErrorMessageOr<std::string> source_file_contents_or_error =
      orbit_base::ReadFileToString(source_file_path);
  ASSERT_TRUE(source_file_contents_or_error.has_value())
      << source_file_contents_or_error.error().message();
  std::string source_file_contents = std::move(source_file_contents_or_error.value());

  orbit_client_data::FunctionInfo function_info{
      "line_info_test_binary",          "buildid", /*address=*/0x401140,
      kMainFunctionInstructions.size(), "main",    /*is_hotpatchable=*/false};

  orbit_code_report::Disassembler disassembler{};
  orbit_client_data::ProcessData process_data{};
  orbit_client_data::ModuleManager module_manager{};
  disassembler.Disassemble(process_data, module_manager,
                           static_cast<const void*>(kMainFunctionInstructions.data()),
                           kMainFunctionInstructions.size(), 0x401140, true);
  std::string assembly = disassembler.GetResult();
  orbit_code_report::DisassemblyReport report{std::move(disassembler), kAddressOfMainFunction};

  orbit_qt::AnnotatingSourceCodeDialog dialog{};
  auto syntax_highlighter = std::make_unique<orbit_syntax_highlighter::X86Assembly>();
  dialog.SetMainContent(QString::fromStdString(assembly), std::move(syntax_highlighter));
  dialog.SetDisassemblyCodeReport(std::move(report));

  bool callback_called = false;
  dialog.AddAnnotatingSourceCode(
      function_info, [&](const orbit_symbol_provider::ModuleIdentifier&) {
        callback_called = true;
        return orbit_base::Future<ErrorMessageOr<std::filesystem::path>>{file_path};
      });

  bool source_code_loaded = false;
  QObject::connect(&dialog, &orbit_qt::AnnotatingSourceCodeDialog::SourceCodeLoaded, &dialog,
                   [&]() {
                     source_code_loaded = true;
                     dialog.close();
                   });

  QObject::connect(&dialog, &orbit_qt::AnnotatingSourceCodeDialog::SourceCodeAvailable, &dialog,
                   [&]() {
                     auto* button = dialog.findChild<QPushButton*>("notification_action_button");

                     if (button == nullptr) return;

                     // If available we will click on the "Load" button to load the source.
                     QTest::mouseClick(button, Qt::LeftButton);
                   });

  QTimer::singleShot(std::chrono::seconds{2}, &dialog, [&]() { dialog.close(); });

  dialog.exec();

  EXPECT_TRUE(callback_called);
  EXPECT_TRUE(source_code_loaded);
}
