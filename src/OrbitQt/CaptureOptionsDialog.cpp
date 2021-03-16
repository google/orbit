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
  ui_->memorySamplingPeriodNsLineEdit->setValidator(&uint64_validator_);
  ui_->memoryWarningThresholdKbLineEdit->setValidator(&uint64_validator_);

  QObject::connect(ui_->openSourcePathsMappingDialog, &QAbstractButton::clicked, this,
                   &CaptureOptionsDialog::ShowSourcePathsMappingEditor);
}

bool CaptureOptionsDialog::GetCollectThreadStates() const {
  return ui_->threadStateCheckBox->isChecked();
}

void CaptureOptionsDialog::SetCollectThreadStates(bool collect_thread_state) {
  ui_->threadStateCheckBox->setChecked(collect_thread_state);
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

void CaptureOptionsDialog::SetCollectMemoryInfo(bool collect_memory_info) {
  ui_->collectMemoryInfoCheckBox->setChecked(collect_memory_info);
}

bool CaptureOptionsDialog::GetCollectMemoryInfo() const {
  return ui_->collectMemoryInfoCheckBox->isChecked();
}

void CaptureOptionsDialog::SetMemorySamplingPeriodNs(uint64_t memory_sampling_period_ns) {
  ui_->memorySamplingPeriodNsLineEdit->setText(QString::number(memory_sampling_period_ns));
}

uint64_t CaptureOptionsDialog::GetMemorySamplingPeriodNs() const {
  CHECK(!ui_->memorySamplingPeriodNsLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t result = ui_->memorySamplingPeriodNsLineEdit->text().toULongLong(&valid);
  CHECK(valid);
  return result;
}

void CaptureOptionsDialog::SetMemoryWarningThresholdKb(uint64_t memory_warning_threshold_kb) {
  ui_->memoryWarningThresholdKbLineEdit->setText(QString::number(memory_warning_threshold_kb));
}

uint64_t CaptureOptionsDialog::GetMemoryWarningThresholdKb() const {
  CHECK(!ui_->memoryWarningThresholdKbLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t result = ui_->memoryWarningThresholdKbLineEdit->text().toULongLong(&valid);
  CHECK(valid);
  return result;
}

}  // namespace orbit_qt
