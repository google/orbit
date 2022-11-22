// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ANNOTATING_SOURCE_CODE_DIALOG_H_
#define ORBIT_QT_ANNOTATING_SOURCE_CODE_DIALOG_H_

#include <absl/types/span.h>

#include <QObject>
#include <QPointer>
#include <QString>
#include <chrono>
#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "CodeReport/AnnotatingLine.h"
#include "CodeReport/DisassemblyReport.h"
#include "CodeViewer/Dialog.h"
#include "CodeViewer/FontSizeInEm.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "OrbitBase/Result.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_qt {

// AnnotatingSourceCodeDialog is an extension of orbit_code_viewer::Dialog which can also manage
// annotating source code which gets loaded asynchronously (while the dialog is already available to
// the user).
//
// The orbit_code_viewer::Dialog's status message box is used to inform the user about the current
// state of source code loading and will be used to ask the user about a source file location if
// needed.
//
// Similar to orbit_code_viewer::OwningDialog this class can manage its own lifetime and be
// destructed when the dialog gets closed. Use the free function `OpenAndDeleteOnClose` to
// obtain this behaviour.
//
// Use `QDialog::exec` if you require a modal dialog which blocks the code path execution while the
// dialog is shown.
class AnnotatingSourceCodeDialog : public orbit_code_viewer::Dialog {
  Q_OBJECT

 public:
  using orbit_code_viewer::Dialog::Dialog;

  // Same interface as OrbitApp::RetrieveModuleWithDebugInfo;
  using RetrieveModuleWithDebugInfoCallback =
      std::function<orbit_base::Future<ErrorMessageOr<std::filesystem::path>>(
          const orbit_symbol_provider::ModuleIdentifier&)>;

  void SetDisassemblyCodeReport(orbit_code_report::DisassemblyReport report) {
    report_ = std::move(report);
  }

  // Enable the heatmap side bar on the left. This requires a disassembly code report to be set with
  // `SetDisassemblyCodeReport`.
  void EnableHeatmap(orbit_code_viewer::FontSizeInEm heatmap_bar_width);

  // Call this function to trigger the annotation process. It requires a disassembly code report to
  // be set before hand by calling `SetDisassemblyCodeReport`.
  void AddAnnotatingSourceCode(orbit_client_data::FunctionInfo function_info,
                               RetrieveModuleWithDebugInfoCallback callback);

 signals:
  void SourceCodeAvailable();
  void SourceCodeLoaded();

 private:
  void DialogActionButtonClicked();
  [[nodiscard]] bool LoadElfFile(const std::filesystem::path& local_file_path);
  [[nodiscard]] bool LoadLocationInformationFromElf();
  [[nodiscard]] bool DetermineLocalSourceFilePath();
  void LoadSourceCode();
  void HandleDebugInfo(const ErrorMessageOr<std::filesystem::path>& local_file_path);
  void ChooseFile();
  void HandleSourceCode(const QString& source_file_contents);
  void HandleAnnotations();

  std::optional<orbit_client_data::FunctionInfo> function_info_;
  std::optional<orbit_code_report::DisassemblyReport> report_;
  RetrieveModuleWithDebugInfoCallback retrieve_module_with_debug_info_;

  std::chrono::steady_clock::time_point starting_time_ = std::chrono::steady_clock::now();
  std::shared_ptr<orbit_base::MainThreadExecutor> main_thread_executor_ =
      orbit_qt_utils::MainThreadExecutorImpl::Create();

  enum class ButtonAction { kNone, kChooseFile, kAddAnnotations, kHide };
  ButtonAction awaited_button_action_ = ButtonAction::kNone;

  std::filesystem::path local_source_file_path_;
  std::unique_ptr<orbit_object_utils::ElfFile> elf_file_;
  orbit_grpc_protos::LineInfo location_info_;
  std::vector<orbit_code_report::AnnotatingLine> annotations_;
};

// This function opens the given dialog and ensures it is deleted when closed.
// Note, this function returns immediately after opening the dialog, NOT when it is closed. Use
// `QDialog::exec` to wait for the dialog.
QPointer<AnnotatingSourceCodeDialog> OpenAndDeleteOnClose(
    std::unique_ptr<AnnotatingSourceCodeDialog> dialog);
}  // namespace orbit_qt

#endif  // ORBIT_QT_ANNOTATING_SOURCE_CODE_DIALOG_H_