// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SourcePathsMappingUI/AskUserForFile.h"

#include <QAbstractButton>
#include <QCheckBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QSettings>
#include <QString>
#include <QVariant>
#include <Qt>
#include <filesystem>
#include <memory>

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

std::optional<UserAnswers> AskUserForSourceFilePath(QWidget* parent,
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

  std::optional<UserAnswers> maybe_user_answers;
  QObject::connect(pick_file_button, &QAbstractButton::clicked, parent, [&]() {
    std::optional<std::filesystem::path> maybe_local_file_path =
        ShowFileOpenDialog(parent, file_path);
    if (!maybe_local_file_path.has_value()) return;

    maybe_user_answers.emplace();
    maybe_user_answers->local_file_path = maybe_local_file_path.value();
    maybe_user_answers->infer_source_paths_mapping = message_box.checkBox()->isChecked();
  });

  message_box.exec();
  return maybe_user_answers;
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