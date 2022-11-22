// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/StopSymbolDownloadDialog.h"

#include <QCheckBox>
#include <QLabel>
#include <memory>

#include "ClientData/ModuleData.h"
#include "OrbitBase/Logging.h"
#include "ui_StopSymbolDownloadDialog.h"

namespace orbit_config_widgets {

StopSymbolDownloadDialog::StopSymbolDownloadDialog(const orbit_client_data::ModuleData* module)
    : ui_(std::make_unique<Ui::StopSymbolDownloadDialog>()) {
  ORBIT_CHECK(module != nullptr);
  ui_->setupUi(this);
  ui_->moduleLabel->setText(QString("<b>%1</b>").arg(QString::fromStdString(module->file_path())));
}

StopSymbolDownloadDialog::~StopSymbolDownloadDialog() = default;

StopSymbolDownloadDialog::Result StopSymbolDownloadDialog::Exec() {
  int rc = exec();
  if (rc != QDialog::Accepted) {
    return Result::kCancel;
  }

  if (ui_->rememberCheckBox->isChecked()) {
    return Result::kStopAndDisable;
  }

  return Result::kStop;
}

}  // namespace orbit_config_widgets