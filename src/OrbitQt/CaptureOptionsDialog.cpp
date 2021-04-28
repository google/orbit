// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureOptionsDialog.h"

#include <absl/flags/declare.h>
#include <absl/flags/flag.h>

#include <QAbstractButton>
#include <QDialog>
#include <QDialogButtonBox>
#include <QWidget>
#include <QtGui/QValidator>

#include "ConfigWidgets/SourcePathsMappingDialog.h"
#include "SourcePathsMapping/MappingManager.h"
#include "ui_CaptureOptionsDialog.h"

ABSL_DECLARE_FLAG(bool, devmode);
ABSL_DECLARE_FLAG(bool, enable_warning_threshold);

namespace orbit_qt {

CaptureOptionsDialog::CaptureOptionsDialog(QWidget* parent)
    : QDialog{parent}, ui_(std::make_unique<Ui::CaptureOptionsDialog>()) {
  ui_->setupUi(this);

  QObject::connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QObject::connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  ui_->localMarkerDepthLineEdit->setValidator(&uint64_validator_);
  ui_->memorySamplingPeriodMsLineEdit->setValidator(new UInt64Validator(1));
  ui_->memoryWarningThresholdKbLineEdit->setValidator(&uint64_validator_);

  QObject::connect(ui_->openSourcePathsMappingDialog, &QAbstractButton::clicked, this,
                   &CaptureOptionsDialog::ShowSourcePathsMappingEditor);

  if (!absl::GetFlag(FLAGS_enable_warning_threshold)) {
    ui_->memoryWarningThresholdKbLabel->hide();
    ui_->memoryWarningThresholdKbLineEdit->hide();
  }

  if (!absl::GetFlag(FLAGS_devmode)) {
    ui_->introspectionCheckBox->hide();
  }
}

bool CaptureOptionsDialog::GetCollectThreadStates() const {
  return ui_->threadStateCheckBox->isChecked();
}

void CaptureOptionsDialog::SetCollectThreadStates(bool collect_thread_state) {
  ui_->threadStateCheckBox->setChecked(collect_thread_state);
}

void CaptureOptionsDialog::SetEnableApi(bool enable_api) {
  ui_->apiCheckBox->setChecked(enable_api);
}

bool CaptureOptionsDialog::GetEnableApi() const { return ui_->apiCheckBox->isChecked(); }

void CaptureOptionsDialog::SetEnableIntrospection(bool enable_introspection) {
  ui_->introspectionCheckBox->setChecked(enable_introspection);
}

bool CaptureOptionsDialog::GetEnableIntrospection() const {
  return ui_->introspectionCheckBox->isChecked();
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

void CaptureOptionsDialog::SetMemorySamplingPeriodMs(uint64_t memory_sampling_period_ms) {
  ui_->memorySamplingPeriodMsLineEdit->setText(QString::number(memory_sampling_period_ms));
}

void CaptureOptionsDialog::ResetMemorySamplingPeriodMsLineEditWhenEmpty() {
  if (!ui_->memorySamplingPeriodMsLineEdit->text().isEmpty()) return;

  constexpr uint64_t kMemorySamplingPeriodMsDefaultValue = 10;
  ui_->memorySamplingPeriodMsLineEdit->setText(
      QString::number(kMemorySamplingPeriodMsDefaultValue));
}

uint64_t CaptureOptionsDialog::GetMemorySamplingPeriodMs() const {
  CHECK(!ui_->memorySamplingPeriodMsLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t memory_sampling_period_ms =
      ui_->memorySamplingPeriodMsLineEdit->text().toULongLong(&valid);
  CHECK(valid);
  return memory_sampling_period_ms;
}

void CaptureOptionsDialog::SetMemoryWarningThresholdKb(uint64_t memory_warning_threshold_kb) {
  ui_->memoryWarningThresholdKbLineEdit->setText(QString::number(memory_warning_threshold_kb));
}

void CaptureOptionsDialog::ResetMemoryWarningThresholdKbLineEditWhenEmpty() {
  if (!ui_->memoryWarningThresholdKbLineEdit->text().isEmpty()) return;

  constexpr uint64_t kMemoryWarningThresholdKbDefaultValue = 1024 * 1024 * 8;
  ui_->memoryWarningThresholdKbLineEdit->setText(
      QString::number(kMemoryWarningThresholdKbDefaultValue));
}

uint64_t CaptureOptionsDialog::GetMemoryWarningThresholdKb() const {
  CHECK(!ui_->memoryWarningThresholdKbLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t result = ui_->memoryWarningThresholdKbLineEdit->text().toULongLong(&valid);
  CHECK(valid);
  return result;
}

}  // namespace orbit_qt
