// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureOptionsDialog.h"

#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <QtGui/QValidator>

#include "ConfigWidgets/SourcePathsMappingDialog.h"
#include "SourcePathsMapping/MappingManager.h"
#include "ui_CaptureOptionsDialog.h"

namespace orbit_qt {

CaptureOptionsDialog::CaptureOptionsDialog(QWidget* parent)
    : QDialog{parent}, ui_(std::make_unique<Ui::CaptureOptionsDialog>()) {
  ui_->setupUi(this);

  QObject::connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QObject::connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  ui_->localMarkerDepthLineEdit->setValidator(&uint64_validator_);

  QObject::connect(ui_->openSourcePathsMappingDialog, &QAbstractButton::clicked, this,
                   &CaptureOptionsDialog::ShowSourcePathsMappingEditor);
}

bool CaptureOptionsDialog::GetCollectThreadStates() const {
  return ui_->threadStateCheckBox->isChecked();
}

bool CaptureOptionsDialog::GetBulkCaptureEvents() const {
  return ui_->bulkCaptureEventsCheckBox->isChecked();
}

void CaptureOptionsDialog::SetCollectThreadStates(bool collect_thread_state) {
  ui_->threadStateCheckBox->setChecked(collect_thread_state);
}

void CaptureOptionsDialog::SetBulkCaptureEvents(bool bulk_capture_events) {
  ui_->bulkCaptureEventsCheckBox->setChecked(bulk_capture_events);
}

void CaptureOptionsDialog::SetLimitLocalMarkerDepthPerCommandBuffer(
    bool limit_local_marker_depth_per_command_buffer) {
  ui_->localMarkerDepthCheckBox->setChecked(limit_local_marker_depth_per_command_buffer);
}

bool CaptureOptionsDialog::GetLimitLocalMarkerDepthPerCommandBuffer() const {
  return ui_->localMarkerDepthCheckBox->isChecked();
}

void CaptureOptionsDialog::SetMaxLocalMarkerDepthPerCommandBuffer(
    uint64_t local_marker_depth_per_command_buffer) {
  ui_->localMarkerDepthLineEdit->setText(QString::number(local_marker_depth_per_command_buffer));
}

uint64_t CaptureOptionsDialog::GetMaxLocalMarkerDepthPerCommandBuffer() const {
  CHECK(!ui_->localMarkerDepthLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t result = ui_->localMarkerDepthLineEdit->text().toULongLong(&valid);
  CHECK(valid);
  return result;
}

void CaptureOptionsDialog::ResetLocalMarkerDepthLineEdit() {
  if (ui_->localMarkerDepthLineEdit->text().isEmpty()) {
    ui_->localMarkerDepthLineEdit->setText(QString::number(0));
  }
}

void CaptureOptionsDialog::ShowSourcePathsMappingEditor() {
  orbit_source_paths_mapping::MappingManager manager{};

  orbit_config_widgets::SourcePathsMappingDialog dialog{this};
  dialog.SetMappings(manager.GetMappings());
  const int result_code = dialog.exec();

  if (result_code == QDialog::Accepted) {
    manager.SetMappings(dialog.GetMappings());
  }
}

}  // namespace orbit_qt
