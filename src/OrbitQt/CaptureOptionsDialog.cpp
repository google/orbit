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

#include "ClientFlags/ClientFlags.h"
#include "ui_CaptureOptionsDialog.h"

namespace orbit_qt {

using orbit_grpc_protos::CaptureOptions;

using DynamicInstrumentationMethod =
    orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod;
using UnwindingMethod = orbit_grpc_protos::CaptureOptions::UnwindingMethod;

CaptureOptionsDialog::CaptureOptionsDialog(QWidget* parent)
    : QDialog{parent}, ui_(std::make_unique<Ui::CaptureOptionsDialog>()) {
  ui_->setupUi(this);

  QObject::connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QObject::connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  QObject::connect(ui_->unwindingMethodComboBox, qOverload<int>(&QComboBox::currentIndexChanged),
                   ui_->maxCopyRawStackSizeLineEdit, [this]() {
                     auto unwinding_method = ui_->unwindingMethodComboBox->currentData().toInt();
                     ui_->maxCopyRawStackSizeLineEdit->setEnabled(unwinding_method ==
                                                                  CaptureOptions::kFramePointers);
                   });
  QObject::connect(ui_->maxCopyRawStackSizeLineEdit, &QLineEdit::editingFinished,
                   ui_->maxCopyRawStackSizeLineEdit, [this]() {
                     if (ui_->maxCopyRawStackSizeLineEdit->text().isEmpty()) {
                       ui_->maxCopyRawStackSizeLineEdit->setText(
                           QString::number(kMaxCopyRawStackSizeDefaultValue));
                     }
                   });

  ui_->unwindingMethodComboBox->addItem("DWARF", static_cast<int>(CaptureOptions::kDwarf));
  ui_->unwindingMethodComboBox->addItem("Frame pointers",
                                        static_cast<int>(CaptureOptions::kFramePointers));

  const QString kFramePointerTooltip =
      "Frame-pointer unwinding is experimental.\n"
      "Successful unwinding requires all binaries to be compiled with\n"
      "'-fno-omit-frame-pointer', but '-momit-leaf-frame-pointer' is\n"
      "also supported.";

  int frame_pointer_item_index =
      ui_->unwindingMethodComboBox->findData(static_cast<int>(CaptureOptions::kFramePointers));
  ui_->unwindingMethodComboBox->setItemData(frame_pointer_item_index, QColor(255, 128, 0, 255),
                                            Qt::ForegroundRole);
  ui_->unwindingMethodComboBox->setItemData(frame_pointer_item_index, kFramePointerTooltip,
                                            Qt::ToolTipRole);

  QObject::connect(
      ui_->unwindingMethodComboBox, &QComboBox::currentTextChanged, ui_->unwindingMethodWidget,
      [this, kFramePointerTooltip]() {
        auto unwinding_method =
            static_cast<UnwindingMethod>(ui_->unwindingMethodComboBox->currentData().toInt());
        if (unwinding_method == CaptureOptions::kFramePointers) {
          ui_->unwindingMethodComboBox->setStyleSheet("QComboBox:editable { color:#ff8000; }");
          ui_->unwindingMethodWidget->setToolTip(kFramePointerTooltip);
        } else {
          ui_->unwindingMethodComboBox->setStyleSheet("QComboBox:editable { color : white; }");
          ui_->unwindingMethodWidget->setToolTip("");
        }
      });

  ui_->maxCopyRawStackSizeLineEdit->setText(QString::number(kMaxCopyRawStackSizeDefaultValue));
  ui_->dynamicInstrumentationMethodComboBox->addItem(
      "Kernel (Uprobes)", static_cast<int>(CaptureOptions::kKernelUprobes));
  ui_->dynamicInstrumentationMethodComboBox->addItem(
      "Orbit", static_cast<int>(CaptureOptions::kUserSpaceInstrumentation));
  if (!absl::GetFlag(FLAGS_devmode)) {
    // TODO(b/198748597): Don't hide samplingCheckBox once disabling sampling completely is exposed.
    ui_->samplingCheckBox->hide();
    ui_->unwindingMethodWidget->hide();
    ui_->maxCopyRawStackSizeWidget->hide();
    ui_->schedulerCheckBox->hide();
    ui_->gpuSubmissionsCheckBox->hide();
    ui_->introspectionCheckBox->hide();
  }

  ui_->localMarkerDepthLineEdit->setValidator(&uint64_validator_);
  ui_->memorySamplingPeriodMsLineEdit->setValidator(new UInt64Validator(
      1, std::numeric_limits<uint64_t>::max(), ui_->memorySamplingPeriodMsLineEdit));
  ui_->memoryWarningThresholdKbLineEdit->setValidator(&uint64_validator_);
  ui_->maxCopyRawStackSizeLineEdit->setValidator(
      new UInt64Validator(0, kMaxCopyRawStackSizeMaxValue, ui_->maxCopyRawStackSizeLineEdit));

  ui_->maxCopyRawStackSizeLabel->setToolTip(QString::fromStdString(
      "To improve performance, we don't require to preserve the frame pointer register on leaf\n"
      "functions (-momit-leaf-frame-pointer). In order to recover the frame corresponding to the\n"
      "caller of a leaf function, we need to copy a portion of the raw stack and DWARF-unwind the\n"
      "first frame. Specify how much data Orbit should copy. A larger value reduces the number of\n"
      "unwind errors, but increases the performance overhead of sampling."));

  if (!absl::GetFlag(FLAGS_enable_warning_threshold)) {
    ui_->memoryWarningThresholdKbLabel->hide();
    ui_->memoryWarningThresholdKbLineEdit->hide();
  }
}

void CaptureOptionsDialog::SetEnableSampling(bool enable_sampling) {
  ui_->samplingCheckBox->setChecked(enable_sampling);
}

bool CaptureOptionsDialog::GetEnableSampling() const { return ui_->samplingCheckBox->isChecked(); }

void CaptureOptionsDialog::SetSamplingPeriodMs(double sampling_period_ms) {
  ui_->samplingPeriodMsDoubleSpinBox->setValue(sampling_period_ms);
}

double CaptureOptionsDialog::GetSamplingPeriodMs() const {
  return ui_->samplingPeriodMsDoubleSpinBox->value();
}

void CaptureOptionsDialog::SetUnwindingMethod(UnwindingMethod unwinding_method) {
  int index = ui_->unwindingMethodComboBox->findData(static_cast<int>(unwinding_method));
  ORBIT_CHECK(index >= 0);
  return ui_->unwindingMethodComboBox->setCurrentIndex(index);
}

UnwindingMethod CaptureOptionsDialog::GetUnwindingMethod() const {
  return static_cast<UnwindingMethod>(ui_->unwindingMethodComboBox->currentData().toInt());
}

void CaptureOptionsDialog::SetMaxCopyRawStackSize(uint16_t stack_dump_size) {
  ui_->maxCopyRawStackSizeLineEdit->setText(QString::number(stack_dump_size));
}

uint16_t CaptureOptionsDialog::GetMaxCopyRawStackSize() const {
  ORBIT_CHECK(!ui_->maxCopyRawStackSizeLineEdit->text().isEmpty());
  bool valid = false;
  uint16_t result = ui_->maxCopyRawStackSizeLineEdit->text().toUShort(&valid);
  ORBIT_CHECK(valid);
  return result;
}

void CaptureOptionsDialog::SetCollectSchedulerInfo(bool collect_scheduler_info) {
  ui_->schedulerCheckBox->setChecked(collect_scheduler_info);
}

bool CaptureOptionsDialog::GetCollectSchedulerInfo() const {
  return ui_->schedulerCheckBox->isChecked();
}

void CaptureOptionsDialog::SetCollectThreadStates(bool collect_thread_state) {
  ui_->threadStateCheckBox->setChecked(collect_thread_state);
}

bool CaptureOptionsDialog::GetCollectThreadStates() const {
  return ui_->threadStateCheckBox->isChecked();
}

void CaptureOptionsDialog::SetTraceGpuSubmissions(bool trace_gpu_submissions) {
  ui_->gpuSubmissionsCheckBox->setChecked(trace_gpu_submissions);
}

bool CaptureOptionsDialog::GetTraceGpuSubmissions() const {
  return ui_->gpuSubmissionsCheckBox->isChecked();
}

void CaptureOptionsDialog::SetEnableApi(bool enable_api) {
  ui_->apiCheckBox->setChecked(enable_api);
}

bool CaptureOptionsDialog::GetEnableApi() const { return ui_->apiCheckBox->isChecked(); }

void CaptureOptionsDialog::SetDynamicInstrumentationMethod(DynamicInstrumentationMethod method) {
  const int index = ui_->dynamicInstrumentationMethodComboBox->findData(static_cast<int>(method));
  ORBIT_CHECK(index >= 0);
  ui_->dynamicInstrumentationMethodComboBox->setCurrentIndex(index);
}

DynamicInstrumentationMethod CaptureOptionsDialog::GetDynamicInstrumentationMethod() const {
  return static_cast<DynamicInstrumentationMethod>(
      ui_->dynamicInstrumentationMethodComboBox->currentData().toInt());
}

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
  ORBIT_CHECK(!ui_->localMarkerDepthLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t result = ui_->localMarkerDepthLineEdit->text().toULongLong(&valid);
  ORBIT_CHECK(valid);
  return result;
}

void CaptureOptionsDialog::ResetLocalMarkerDepthLineEdit() {
  if (ui_->localMarkerDepthLineEdit->text().isEmpty()) {
    ui_->localMarkerDepthLineEdit->setText(QString::number(kLocalMarkerDepthDefaultValue));
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
  ui_->memorySamplingPeriodMsLineEdit->setText(
      QString::number(kMemorySamplingPeriodMsDefaultValue));
}

uint64_t CaptureOptionsDialog::GetMemorySamplingPeriodMs() const {
  ORBIT_CHECK(!ui_->memorySamplingPeriodMsLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t memory_sampling_period_ms =
      ui_->memorySamplingPeriodMsLineEdit->text().toULongLong(&valid);
  ORBIT_CHECK(valid);
  return memory_sampling_period_ms;
}

void CaptureOptionsDialog::SetMemoryWarningThresholdKb(uint64_t memory_warning_threshold_kb) {
  ui_->memoryWarningThresholdKbLineEdit->setText(QString::number(memory_warning_threshold_kb));
}

void CaptureOptionsDialog::ResetMemoryWarningThresholdKbLineEditWhenEmpty() {
  if (!ui_->memoryWarningThresholdKbLineEdit->text().isEmpty()) return;
  ui_->memoryWarningThresholdKbLineEdit->setText(
      QString::number(kMemoryWarningThresholdKbDefaultValue));
}

uint64_t CaptureOptionsDialog::GetMemoryWarningThresholdKb() const {
  ORBIT_CHECK(!ui_->memoryWarningThresholdKbLineEdit->text().isEmpty());
  bool valid = false;
  uint64_t result = ui_->memoryWarningThresholdKbLineEdit->text().toULongLong(&valid);
  ORBIT_CHECK(valid);
  return result;
}

}  // namespace orbit_qt
