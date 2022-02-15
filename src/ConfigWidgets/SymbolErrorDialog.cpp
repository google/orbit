// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/SymbolErrorDialog.h"

#include <QDialog>
#include <QPushButton>
#include <memory>

#include "ConfigWidgets/SymbolsDialog.h"
#include "OrbitBase/Logging.h"
#include "SymbolPaths/PersistentStorageManager.h"
#include "ui_SymbolErrorDialog.h"

namespace orbit_config_widgets {

SymbolErrorDialog::SymbolErrorDialog(const orbit_client_data::ModuleData* module,
                                     const std::string& detailed_error, QWidget* parent)
    : QDialog(parent), ui_(std::make_unique<Ui::SymbolErrorDialog>()), module_(module) {
  ORBIT_CHECK(module_ != nullptr);
  ui_->setupUi(this);
  ui_->moduleNameLabel->setText(QString::fromStdString(module_->file_path()));
  ui_->errorPlainTextEdit->setPlainText(QString::fromStdString(detailed_error));
  ui_->addSymbolLocationButton->setFocus();
}

SymbolErrorDialog::~SymbolErrorDialog() = default;

void SymbolErrorDialog::OnShowErrorButtonClicked() {
  ui_->errorPlainTextEdit->setVisible(!ui_->errorPlainTextEdit->isVisible());
  QString button_text =
      ui_->errorPlainTextEdit->isVisible() ? "Hide detailed error" : "Show detailed error";
  ui_->showErrorButton->setText(button_text);
}

void SymbolErrorDialog::OnAddSymbolLocationButtonClicked() {
  result_ = Result::kAddSymbolLocation;
  reject();
}

void SymbolErrorDialog::OnTryAgainButtonClicked() {
  result_ = Result::kTryAgain;
  reject();
}

SymbolErrorDialog::Result SymbolErrorDialog::Exec() {
  exec();
  return result_;
}

}  // namespace orbit_config_widgets