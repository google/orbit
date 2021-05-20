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

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
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
    std::optional<std::filesystem::path> maybe_local_file_path =
        ShowFileOpenDialog(parent, file_path);
    if (!maybe_local_file_path.has_value()) return;

    ErrorMessageOr<std::string> file_contents_or_error =
        orbit_base::ReadFileToString(maybe_local_file_path.value());
    if (file_contents_or_error.has_error()) {
      QMessageBox::critical(parent, "Could not open source file",
                            QString::fromStdString(file_contents_or_error.error().message()));
      return;
    }

    const bool infer_source_paths_mapping = message_box.checkBox()->isChecked();
    if (infer_source_paths_mapping) {
      orbit_source_paths_mapping::InferAndAppendSourcePathsMapping(file_path,
                                                                   maybe_local_file_path.value());
    }

    maybe_file_contents = QString::fromStdString(file_contents_or_error.value());
  });

  message_box.exec();

  if (maybe_file_contents.has_value()) {
    QSettings settings{};
    settings.setValue(kAutocreateMappingKey, message_box.checkBox()->isChecked());
  }

  return maybe_file_contents;
}

std::optional<std::filesystem::path> ShowFileOpenDialog(QWidget* parent,
                                                        const std::filesystem::path& file_path) {
  QSettings settings{};
  QDir previous_directory{
      settings.value(kPreviousSourcePathsMappingDirectoryKey, QDir::currentPath()).toString()};
  const QString file_name = QString::fromStdString(file_path.filename().string());

  QString user_chosen_file = QFileDialog::getOpenFileName(
      parent, QString{"Choose %1"}.arg(QString::fromStdString(file_path.string())),
      previous_directory.filePath(file_name), file_name);

  if (user_chosen_file.isEmpty()) return std::nullopt;

  settings.setValue(kPreviousSourcePathsMappingDirectoryKey, QFileInfo{user_chosen_file}.path());

  return std::filesystem::path{user_chosen_file.toStdString()};
}

}  // namespace orbit_source_paths_mapping_ui