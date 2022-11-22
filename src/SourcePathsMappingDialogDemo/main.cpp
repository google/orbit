// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QApplication>
#include <QDialog>

#include "ConfigWidgets/SourcePathsMappingDialog.h"
#include "SourcePathsMapping/Mapping.h"
#include "SourcePathsMapping/MappingManager.h"

int main(int argc, char* argv[]) {
  QApplication app{argc, argv};
  QApplication::setApplicationName("SourcePathsMappingDialogDemo");
  QApplication::setOrganizationName("The Orbit Authors");

  orbit_source_paths_mapping::MappingManager manager{};

  orbit_config_widgets::SourcePathsMappingDialog dialog{};
  dialog.SetMappings(manager.GetMappings());
  const int result_code = dialog.exec();

  if (result_code == QDialog::Accepted) {
    manager.SetMappings(dialog.GetMappings());
  }

  return result_code;
}