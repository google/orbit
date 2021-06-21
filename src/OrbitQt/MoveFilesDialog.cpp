// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/MoveFilesDialog.h"

#include <QMessageBox>
#include <QPlainTextEdit>
#include <QString>

#include "ui_MoveFilesDialog.h"

namespace orbit_qt {

MoveFilesDialog::MoveFilesDialog() : QDialog(nullptr), ui_(new Ui::MoveFilesDialog) {
  ui_->setupUi(this);
#ifdef WIN32
  ui_->label->setText(
      "We are moving captures and presets from %APPDATA%\\OrbitProfiler to Documents\\Orbit. "
      "Please wait...");
#else
  ui_->label->setText(
      "We are moving captures and presets from ~/.orbitprofiler to ~/Documents/Orbit. "
      "Please wait...");
#endif

  QObject::connect(ui_->button, &QPushButton::clicked, this, [this]() {
    switch (status_) {
      case Status::kInProgress:
        ShowRequestInterruptionConfirmation();
        break;
      case Status::kInterruptionRequested:
        break;
      case Status::kDone:
        accept();
        break;
    }
  });
}

MoveFilesDialog::~MoveFilesDialog() noexcept = default;

void MoveFilesDialog::AddText(std::string_view text) {
  ui_->log->append(QString::fromUtf8(text.data(), text.size()));
}

void MoveFilesDialog::OnMoveFinished() {
  status_ = Status::kDone;
  ui_->button->setText("Close");
  ui_->button->setEnabled(true);
  ui_->button->setDefault(true);
}

void MoveFilesDialog::OnMoveInterrupted() {
  status_ = Status::kDone;
  this->reject();
}

void MoveFilesDialog::closeEvent(QCloseEvent* event) {
  switch (status_) {
    case Status::kInProgress:
      ShowRequestInterruptionConfirmation();
      event->ignore();
      break;
    case Status::kInterruptionRequested:
      event->ignore();
      break;
    case Status::kDone:
      event->accept();
      break;
  }
}

void MoveFilesDialog::keyPressEvent(QKeyEvent* event) {
  // The Escape key doesn't trigger closeEvent, we have to handle it separately.
  if (event->key() != Qt::Key_Escape) {
    QDialog::keyPressEvent(event);
    return;
  }

  switch (status_) {
    case Status::kInProgress:
      ShowRequestInterruptionConfirmation();
      event->ignore();
      break;
    case Status::kInterruptionRequested:
      event->ignore();
      break;
    case Status::kDone:
      QDialog::keyPressEvent(event);
      break;
  }
}

void MoveFilesDialog::ShowRequestInterruptionConfirmation() {
  QMessageBox::StandardButton result = QMessageBox::question(
      this, "Move is in progress",
      "We are still moving some files. The move will be suspended, but will continue the next "
      "time you open Orbit. You will still need to wait for the current file to finish being "
      "moved.\n\nAre you sure you want to skip moving the remaining files for now?",
      QMessageBox::StandardButtons{QMessageBox::StandardButton::Yes |
                                   QMessageBox::StandardButton::No},
      QMessageBox::StandardButton::No);
  if (result == QMessageBox::StandardButton::Yes) {
    status_ = Status::kInterruptionRequested;
    ui_->button->setText("Suspending...");
    ui_->button->setEnabled(false);
    emit interruptionRequested();
  }
}

}  // namespace orbit_qt
