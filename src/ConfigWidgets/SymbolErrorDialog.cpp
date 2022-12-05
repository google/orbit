// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/SymbolErrorDialog.h"

#include <absl/flags/flag.h>

#include <QDialog>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <memory>
#include <string>

#include "ClientFlags/ClientFlags.h"
#include "OrbitBase/Logging.h"
#include "ui_SymbolErrorDialog.h"

namespace orbit_config_widgets {

SymbolErrorDialog::SymbolErrorDialog(const orbit_client_data::ModuleData* module,
                                     std::string_view detailed_error, QWidget* parent)
    : QDialog(parent), ui_(std::make_unique<Ui::SymbolErrorDialog>()), module_(module) {
  ORBIT_CHECK(module_ != nullptr);
  ui_->setupUi(this);
  QString module_file_path = QString::fromStdString(module_->file_path());
  ui_->moduleNameLabel->setText(module_file_path);
  ui_->errorPlainTextEdit->setPlainText(
      QString::fromUtf8(detailed_error.data(), detailed_error.size()));

  // If the module has a build id, or unsafe symbols are allowed, then the user can continue to
  // resolve this error.
  if (!module_->build_id().empty() || absl::GetFlag(FLAGS_enable_unsafe_symbols)) {
    ui_->addSymbolLocationButton->setFocus();
    return;
  }

  // The module does not have a build id, and only safe symbols are allowed, therefore the user
  // cannot resolve this error.
  ui_->addSymbolLocationButton->setEnabled(false);
  ui_->addSymbolLocationButton->setToolTip(
      QString("Orbit matches modules and symbol files based on build-id. Module %1 does not "
              "contain a build id.")
          .arg(module_file_path));
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