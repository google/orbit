// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SOURCE_PATHS_MAPPING_UI_ASK_USER_FOR_FILE_H_
#define SOURCE_PATHS_MAPPING_UI_ASK_USER_FOR_FILE_H_

#include <QString>
#include <QWidget>
#include <filesystem>
#include <optional>

#include "OrbitBase/Logging.h"

namespace orbit_source_paths_mapping_ui {

// Shows a dialog explaining to the user that the file from `file_path` hasn't been found on their
// machine. The user now has to choice to abort the operation or choose a file manually with a
// file-open-dialog.
// If the users chooses the latter the file will be read and the contents returned as a QString.
//
// The user also has the choice to infer a source paths mapping from their chosen file path.
std::optional<QString> TryAskingTheUserAndReadSourceFile(QWidget* parent,
                                                         const std::filesystem::path& file_path);

// Shows a file open dialog and returns the contents of the chosen file. It will also try to create
// a source paths mapping when `infer_source_paths_mapping` is set to true. If reading the file
// fails it will return an ErrorMessage. If the user aborts it will return std::nullopt. Otherwise
// it returns the contents as a QString.
ErrorMessageOr<std::optional<QString>> ShowFileOpenDialogAndReadSourceFile(
    QWidget* parent, const std::filesystem::path& file_path, bool infer_source_paths_mapping);

}  // namespace orbit_source_paths_mapping_ui

#endif  // SOURCE_PATHS_MAPPING_UI_ASK_USER_FOR_FILE_H_