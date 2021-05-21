// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SourcePathsMappingUI/AskUserForFile.h"

#include <absl/strings/str_format.h>

#include <QAbstractButton>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <filesystem>
#include <memory>

#include "OrbitBase/ReadFileToString.h"
#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingManager.h"

namespace orbit_source_paths_mapping_ui {
constexpr const char* kAutocreateMappingKey = "auto_create_mapping";
constexpr const char* kPreviousSourcePathsMappingDirectoryKey =
    "previous_source_paths_mapping_directory";

static std::unique_ptr<QCheckBox> CreateSourcePathsMappingCheckBox() {
  std::unique_ptr<QCheckBox> check_box = std::make_unique<QCheckBox>(
      "Automatically create a source paths mapping from my selected file.");
  check_box->setToolTip(
      "If enabled, Orbit will automatically try to create a source paths mapping from it. The "
      "common suffix between the path given in the debug information and the local file path will "
      "be stripped. From the rest a mapping will be created.");
  QSettings settings{};
  check_box->setCheckState(settings.value(kAutocreateMappingKey, true).toBool() ? Qt::Checked
                                                                                : Qt::Unchecked);
  return check_box;
}

std::optional<QString> TryAskingTheUserAndReadSourceFile(QWidget* parent,
                                                         const std::filesystem::path& file_path) {
  QMessageBox message_box{QMessageBox::Warning, "Source code file not found",
                          QString("Could not find the source code file \"%1\" on this machine.")
                              .arg(QString::fromStdString(file_path.string())),
                          QMessageBox::Cancel, parent};
  QPushButton* pick_file_button = message_box.addButton("Choose file...", QMessageBox::ActionRole);

  {
    std::unique_ptr<QCheckBox> check_box = CreateSourcePathsMappingCheckBox();
    // Ownership will be transferred to message_box
    message_box.setCheckBox(check_box.release());
  }

  std::optional<QString> maybe_file_contents;
  QObject::connect(pick_file_button, &QAbstractButton::clicked, parent, [&]() {
    const bool infer_source_paths_mapping = message_box.checkBox()->isChecked();
    ErrorMessageOr<std::optional<QString>> result =
        ShowFileOpenDialogAndReadSourceFile(parent, file_path, infer_source_paths_mapping);
    if (result.has_error()) {
      QMessageBox::critical(parent, "Could not open source file",
                            QString::fromStdString(result.error().message()));
      return;
    }

    maybe_file_contents = std::move(result.value());
  });

  message_box.exec();

  if (maybe_file_contents.has_value()) {
    QSettings settings{};
    settings.setValue(kAutocreateMappingKey, message_box.checkBox()->isChecked());
  }

  return maybe_file_contents;
}

ErrorMessageOr<std::optional<QString>> ShowFileOpenDialogAndReadSourceFile(
    QWidget* parent, const std::filesystem::path& file_path, bool infer_source_paths_mapping) {
  QSettings settings{};
  QDir previous_directory{
      settings.value(kPreviousSourcePathsMappingDirectoryKey, QDir::currentPath()).toString()};
  const QString file_name = QString::fromStdString(file_path.filename().string());

  QString user_chosen_file = QFileDialog::getOpenFileName(
      parent, QString{"Choose %1"}.arg(QString::fromStdString(file_path.string())),
      previous_directory.filePath(file_name), file_name);

  if (user_chosen_file.isEmpty()) return std::nullopt;

  settings.setValue(kPreviousSourcePathsMappingDirectoryKey, QFileInfo{user_chosen_file}.path());

  ErrorMessageOr<std::string> maybe_source_code =
      orbit_base::ReadFileToString(std::filesystem::path{user_chosen_file.toStdString()});
  if (maybe_source_code.has_error()) {
    return ErrorMessage{absl::StrFormat("Could not open file \"%s\" for reading: %s",
                                        user_chosen_file.toStdString(),
                                        maybe_source_code.error().message())};
  }

  if (infer_source_paths_mapping) {
    std::optional<orbit_source_paths_mapping::Mapping> maybe_mapping =
        orbit_source_paths_mapping::InferMappingFromExample(
            file_path, std::filesystem::path{user_chosen_file.toStdString()});
    if (maybe_mapping.has_value()) {
      orbit_source_paths_mapping::MappingManager mapping_manager{};
      mapping_manager.AppendMapping(maybe_mapping.value());
    } else {
      LOG("Unable to infer a mapping from \"%s\" to \"%s\"", file_path.string(),
          user_chosen_file.toStdString());
    }
  }

  return QString::fromStdString(maybe_source_code.value());
}

}  // namespace orbit_source_paths_mapping_ui