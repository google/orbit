// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/AnnotatingSourceCodeDialog.h"

#include <Qt>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <ratio>
#include <string>

#include "CodeReport/AnnotateDisassembly.h"
#include "CodeReport/AnnotatingLine.h"
#include "ObjectUtils/ElfFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "SourcePathsMapping/MappingManager.h"
#include "SourcePathsMappingUI/AskUserForFile.h"

namespace orbit_qt {
void AnnotatingSourceCodeDialog::AddAnnotatingSourceCode(
    orbit_client_data::FunctionInfo function_info, RetrieveModuleWithDebugInfoCallback callback) {
  function_info_ = std::move(function_info);
  retrieve_module_with_debug_info_ = std::move(callback);

  QObject::connect(this, &orbit_code_viewer::Dialog::StatusMessageButtonClicked, this,
                   &AnnotatingSourceCodeDialog::DialogActionButtonClicked);

  SetStatusMessage("Loading source location information", std::nullopt);

  ORBIT_CHECK(function_info_.has_value());
  retrieve_module_with_debug_info_(function_info_->module_id())
      .Then(main_thread_executor_.get(),
            [this](const ErrorMessageOr<std::filesystem::path>& local_file_path_or_error) {
              HandleDebugInfo(local_file_path_or_error);
            });
}

void AnnotatingSourceCodeDialog::EnableHeatmap(orbit_code_viewer::FontSizeInEm heatmap_bar_width) {
  if (!report_.has_value()) return;

  SetHeatmap(heatmap_bar_width, &report_.value());
}

void AnnotatingSourceCodeDialog::DialogActionButtonClicked() {
  switch (awaited_button_action_) {
    case ButtonAction::kNone:
      break;
    case ButtonAction::kChooseFile:
      ChooseFile();
      break;
    case ButtonAction::kAddAnnotations:
      HandleAnnotations();
      break;
    case ButtonAction::kHide:
      ClearStatusMessage();
      break;
  }
}

bool AnnotatingSourceCodeDialog::LoadElfFile(const std::filesystem::path& local_file_path) {
  ErrorMessageOr<std::unique_ptr<orbit_object_utils::ElfFile>> elf_or_error =
      orbit_object_utils::CreateElfFile(local_file_path);
  if (elf_or_error.has_error()) {
    SetStatusMessage(QString::fromStdString(elf_or_error.error().message()), "Hide");
    awaited_button_action_ = ButtonAction::kHide;
    return false;
  }
  elf_file_ = std::move(elf_or_error.value());
  return true;
}
bool AnnotatingSourceCodeDialog::LoadLocationInformationFromElf() {
  ORBIT_CHECK(function_info_.has_value());
  ErrorMessageOr<orbit_grpc_protos::LineInfo> location_or_error =
      elf_file_->GetLocationOfFunction(function_info_->address());
  if (location_or_error.has_error()) {
    SetStatusMessage(QString::fromStdString(location_or_error.error().message()), "Hide");
    awaited_button_action_ = ButtonAction::kHide;
    return false;
  }
  location_info_ = std::move(location_or_error.value());
  return true;
}

bool AnnotatingSourceCodeDialog::DetermineLocalSourceFilePath() {
  local_source_file_path_ = std::filesystem::path{location_info_.source_file()};

  if (!std::filesystem::exists(local_source_file_path_)) {
    orbit_source_paths_mapping::MappingManager mapping_manager{};
    std::optional<std::filesystem::path> maybe_mapping_file_path =
        mapping_manager.MapToFirstExistingTarget(local_source_file_path_);
    if (!maybe_mapping_file_path.has_value()) {
      SetStatusMessage(QString("Could not find the source code file \"%1\" on this machine.")
                           .arg(QString::fromStdString(local_source_file_path_.string())),
                       "Choose file...");
      awaited_button_action_ = ButtonAction::kChooseFile;
      return false;
    }

    local_source_file_path_ = std::move(maybe_mapping_file_path.value());
  }

  return true;
}

void AnnotatingSourceCodeDialog::LoadSourceCode() {
  ErrorMessageOr<std::string> source_file_contents_or_error =
      orbit_base::ReadFileToString(local_source_file_path_);

  if (source_file_contents_or_error.has_error()) {
    SetStatusMessage(
        QString("Error while reading source code file \"%1\":\n%2")
            .arg(QString::fromStdString(local_source_file_path_.string()),
                 QString::fromStdString(source_file_contents_or_error.error().message())),
        "Choose another file...");
    awaited_button_action_ = ButtonAction::kChooseFile;
    return;
  }

  HandleSourceCode(QString::fromStdString(source_file_contents_or_error.value()));
}

void AnnotatingSourceCodeDialog::HandleDebugInfo(
    const ErrorMessageOr<std::filesystem::path>& local_file_path_or_error) {
  if (local_file_path_or_error.has_error()) {
    ORBIT_LOG("Error while loading debug information for the disassembly view: %s",
              local_file_path_or_error.error().message());

    SetStatusMessage(QString::fromStdString(local_file_path_or_error.error().message()), "Hide");
    awaited_button_action_ = ButtonAction::kHide;

    return;
  }

  const std::filesystem::path& local_file_path = local_file_path_or_error.value();
  if (!LoadElfFile(local_file_path)) return;
  if (!LoadLocationInformationFromElf()) return;
  if (!DetermineLocalSourceFilePath()) return;
  LoadSourceCode();
}

void AnnotatingSourceCodeDialog::ChooseFile() {
  std::optional<std::filesystem::path> maybe_local_file_path =
      orbit_source_paths_mapping_ui::ShowFileOpenDialog(this, local_source_file_path_);

  // The user aborted the operation. We change nothing in that case.
  if (!maybe_local_file_path.has_value()) return;

  ErrorMessageOr<std::string> file_contents_or_error =
      orbit_base::ReadFileToString(maybe_local_file_path.value());

  if (file_contents_or_error.has_error()) {
    SetStatusMessage(QString::fromStdString(file_contents_or_error.error().message()),
                     "Choose another file...");
    return;
  }

  orbit_source_paths_mapping::InferAndAppendSourcePathsMapping(local_source_file_path_,
                                                               maybe_local_file_path.value());

  HandleSourceCode(QString::fromStdString(file_contents_or_error.value()));
}

void AnnotatingSourceCodeDialog::HandleSourceCode(const QString& source_file_contents) {
  ORBIT_CHECK(function_info_.has_value());
  annotations_ = orbit_code_report::AnnotateDisassemblyWithSourceCode(
      *function_info_, location_info_, source_file_contents.toStdString(), elf_file_.get(),
      report_.value());

  // When loading the source code takes less than the given time we won't ask the user if they want
  // to have the source code added for context. We will add it right away - as it's done in Visual
  // Studio.
  constexpr std::chrono::steady_clock::duration kMaxWaitingTime = std::chrono::milliseconds{250};

  // ButtonAction::kChooseFile means the user had to select the source file path manually. In this
  // case we don't keep them waiting even if loading took longer than the limit.
  if (awaited_button_action_ == ButtonAction::kChooseFile ||
      std::chrono::steady_clock::now() - starting_time_ < kMaxWaitingTime) {
    HandleAnnotations();
    return;
  }

  SetStatusMessage("Source code annotations are available now.", "Load");
  awaited_button_action_ = ButtonAction::kAddAnnotations;
  emit SourceCodeAvailable();
}

void AnnotatingSourceCodeDialog::HandleAnnotations() {
  SetAnnotatingContent(annotations_);
  annotations_.clear();

  ClearStatusMessage();
  awaited_button_action_ = ButtonAction::kNone;

  emit SourceCodeLoaded();
}

QPointer<AnnotatingSourceCodeDialog> OpenAndDeleteOnClose(
    std::unique_ptr<AnnotatingSourceCodeDialog> dialog) {
  dialog->open();
  dialog->setAttribute(Qt::WA_DeleteOnClose);
  return QPointer<AnnotatingSourceCodeDialog>{dialog.release()};
}

}  // namespace orbit_qt
