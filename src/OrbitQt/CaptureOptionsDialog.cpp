// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/CaptureOptionsDialog.h"

#include <absl/flags/flag.h>

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QNonConstOverload>
#include <QRadioButton>
#include <QWidget>

#include "ClientFlags/ClientFlags.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitQt/MultipleOfSpinBox.h"
#include "ui_CaptureOptionsDialog.h"

namespace orbit_qt {

using orbit_client_data::WineSyscallHandlingMethod;
using orbit_grpc_protos::CaptureOptions;

using DynamicInstrumentationMethod =
    orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod;
using UnwindingMethod = orbit_grpc_protos::CaptureOptions::UnwindingMethod;

CaptureOptionsDialog::CaptureOptionsDialog(QWidget* parent)
    : QDialog{parent}, ui_(std::make_unique<Ui::CaptureOptionsDialog>()) {
  ui_->setupUi(this);

  QObject::connect(ui_->buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  QObject::connect(ui_->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  QObject::connect(ui_->framePointerUnwindingRadioButton, qOverload<bool>(&QRadioButton::toggled),
                   ui_->maxCopyRawStackSizeWidget,
                   [this](bool checked) { ui_->maxCopyRawStackSizeWidget->setEnabled(checked); });
  QObject::connect(ui_->dwarfUnwindingRadioButton, qOverload<bool>(&QRadioButton::toggled),
                   ui_->wineGroupBox,
                   [this](bool checked) { ui_->wineGroupBox->setEnabled(checked); });
  QObject::connect(ui_->collectMemoryInfoCheckBox, qOverload<bool>(&QCheckBox::toggled), this,
                   [this](bool checked) {
                     ui_->memorySamplingPeriodMsLabel->setEnabled(checked);
                     ui_->memorySamplingPeriodMsLineEdit->setEnabled(checked);
                     ui_->memoryWarningThresholdKbLabel->setEnabled(checked);
                     ui_->memoryWarningThresholdKbLineEdit->setEnabled(checked);
                   });

  QObject::connect(ui_->samplingCheckBox, qOverload<bool>(&QCheckBox::toggled), this,
                   [this](bool checked) {
                     ui_->samplingPeriodMsLabel->setEnabled(checked);
                     ui_->samplingPeriodMsDoubleSpinBox->setEnabled(checked);
                     ui_->unwindingMethodGroupBox->setEnabled(checked);
                   });

  QObject::connect(ui_->threadStateCheckBox, qOverload<bool>(&QCheckBox::toggled),
                   ui_->threadStateChangeCallstackCollectionCheckBox, &QCheckBox::setEnabled);
  QObject::connect(ui_->threadStateCheckBox, qOverload<bool>(&QCheckBox::toggled),
                   ui_->threadStateChangeCallstackMaxCopyRawStackSizeWidget, [this](bool checked) {
                     ui_->threadStateChangeCallstackMaxCopyRawStackSizeWidget->setEnabled(
                         checked && ui_->threadStateChangeCallstackCollectionCheckBox->isChecked());
                   });
  QObject::connect(ui_->threadStateChangeCallstackCollectionCheckBox,
                   qOverload<bool>(&QCheckBox::toggled),
                   ui_->threadStateChangeCallstackMaxCopyRawStackSizeWidget, [this](bool checked) {
                     ui_->threadStateChangeCallstackMaxCopyRawStackSizeWidget->setEnabled(
                         checked && ui_->threadStateCheckBox->isChecked());
                   });

  ui_->samplingPeriodMsLabel->setEnabled(ui_->samplingCheckBox->isChecked());
  ui_->samplingPeriodMsDoubleSpinBox->setEnabled(ui_->samplingCheckBox->isChecked());
  ui_->unwindingMethodGroupBox->setEnabled(ui_->samplingCheckBox->isChecked());

  ui_->maxCopyRawStackSizeSpinBox->setValue(kMaxCopyRawStackSizeDefaultValue);
  ui_->maxCopyRawStackSizeWidget->setEnabled(ui_->framePointerUnwindingRadioButton->isChecked());

  ui_->wineGroupBox->setEnabled(ui_->dwarfUnwindingRadioButton->isChecked());

  ui_->localMarkerDepthLineEdit->setValidator(&uint64_validator_);

  ui_->memorySamplingPeriodMsLabel->setEnabled(ui_->collectMemoryInfoCheckBox->isChecked());
  ui_->memorySamplingPeriodMsLineEdit->setEnabled(ui_->collectMemoryInfoCheckBox->isChecked());
  ui_->memoryWarningThresholdKbLabel->setEnabled(ui_->collectMemoryInfoCheckBox->isChecked());
  ui_->memoryWarningThresholdKbLineEdit->setEnabled(ui_->collectMemoryInfoCheckBox->isChecked());
  ui_->memorySamplingPeriodMsLineEdit->setValidator(new UInt64Validator(
      1, std::numeric_limits<uint64_t>::max(), ui_->memorySamplingPeriodMsLineEdit));
  ui_->memoryWarningThresholdKbLineEdit->setValidator(&uint64_validator_);
  ui_->threadStateChangeCallstackCollectionCheckBox->setEnabled(
      ui_->threadStateCheckBox->isChecked());

  ui_->threadStateChangeCallstackMaxCopyRawStackSizeSpinBox->setValue(
      kThreadStateChangeMaxCopyRawStackSizeDefaultValue);
  ui_->threadStateChangeCallstackMaxCopyRawStackSizeWidget->setEnabled(
      ui_->threadStateChangeCallstackCollectionCheckBox->isChecked() &&
      ui_->threadStateCheckBox->isChecked());

  if (!absl::GetFlag(FLAGS_auto_frame_track)) {
    ui_->autoFrameTrackGroupBox->hide();
  }

  if (!absl::GetFlag(FLAGS_enable_warning_threshold)) {
    ui_->memoryWarningThresholdKbLabel->hide();
    ui_->memoryWarningThresholdKbLineEdit->hide();
  }

  if (!absl::GetFlag(FLAGS_devmode)) {
    // TODO(b/198748597): Don't hide samplingCheckBox once disabling sampling completely is exposed.
    ui_->samplingCheckBox->hide();
    ui_->schedulerCheckBox->hide();
    ui_->devModeGroupBox->hide();
    ui_->wineNoneRadioButton->hide();
  }
}

CaptureOptionsDialog::~CaptureOptionsDialog() = default;

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
  switch (unwinding_method) {
    case CaptureOptions::kDwarf:
      ui_->dwarfUnwindingRadioButton->setChecked(true);
      break;
    case CaptureOptions::kFramePointers:
      ui_->framePointerUnwindingRadioButton->setChecked(true);
      break;
    default:
      ORBIT_UNREACHABLE();
  }
}

UnwindingMethod CaptureOptionsDialog::GetUnwindingMethod() const {
  if (ui_->dwarfUnwindingRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->framePointerUnwindingRadioButton->isChecked());
    return CaptureOptions::kDwarf;
  }
  if (ui_->framePointerUnwindingRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->dwarfUnwindingRadioButton->isChecked());
    return CaptureOptions::kFramePointers;
  }
  ORBIT_UNREACHABLE();
}

void CaptureOptionsDialog::SetMaxCopyRawStackSize(uint16_t stack_dump_size) {
  ORBIT_CHECK(stack_dump_size % 8 == 0);
  ui_->maxCopyRawStackSizeSpinBox->setValue(stack_dump_size);
}

uint16_t CaptureOptionsDialog::GetMaxCopyRawStackSize() const {
  uint16_t result = ui_->maxCopyRawStackSizeSpinBox->value();
  ORBIT_CHECK(result % 8 == 0);
  return result;
}

void CaptureOptionsDialog::SetThreadStateChangeCallstackMaxCopyRawStackSize(
    uint16_t stack_dump_size) {
  ORBIT_CHECK(stack_dump_size % 8 == 0);
  ui_->threadStateChangeCallstackMaxCopyRawStackSizeSpinBox->setValue(stack_dump_size);
}

uint16_t CaptureOptionsDialog::GetThreadStateChangeCallstackMaxCopyRawStackSize() const {
  uint16_t result = ui_->threadStateChangeCallstackMaxCopyRawStackSizeSpinBox->value();
  ORBIT_CHECK(result % 8 == 0);
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
  switch (method) {
    case CaptureOptions::kKernelUprobes:
      ui_->uprobesRadioButton->setChecked(true);
      break;
    case CaptureOptions::kUserSpaceInstrumentation:
      ui_->userSpaceRadioButton->setChecked(true);
      break;
    default:
      ORBIT_UNREACHABLE();
  }
}

DynamicInstrumentationMethod CaptureOptionsDialog::GetDynamicInstrumentationMethod() const {
  if (ui_->uprobesRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->userSpaceRadioButton->isChecked());
    return CaptureOptions::kKernelUprobes;
  }
  if (ui_->userSpaceRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->uprobesRadioButton->isChecked());
    return CaptureOptions::kUserSpaceInstrumentation;
  }
  ORBIT_UNREACHABLE();
}

void CaptureOptionsDialog::SetEnableCallStackCollectionOnThreadStateChanges(bool check) {
  ui_->threadStateChangeCallstackCollectionCheckBox->setChecked(check);
}

bool CaptureOptionsDialog::GetEnableCallStackCollectionOnThreadStateChanges() const {
  return ui_->threadStateChangeCallstackCollectionCheckBox->isChecked();
}

void CaptureOptionsDialog::SetWineSyscallHandlingMethod(
    orbit_client_data::WineSyscallHandlingMethod method) {
  switch (method) {
    case WineSyscallHandlingMethod::kNoSpecialHandling:
      ui_->wineNoneRadioButton->setChecked(true);
      break;
    case WineSyscallHandlingMethod::kStopUnwinding:
      ui_->wineStopRadioButton->setChecked(true);
      break;
    case WineSyscallHandlingMethod::kRecordUserStack:
      ui_->wineRecordRadioButton->setChecked(true);
      break;
  }
}

WineSyscallHandlingMethod CaptureOptionsDialog::GetWineSyscallHandlingMethod() const {
  if (ui_->wineNoneRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->wineStopRadioButton->isChecked() && !ui_->wineRecordRadioButton->isChecked());
    return WineSyscallHandlingMethod::kNoSpecialHandling;
  }
  if (ui_->wineStopRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->wineNoneRadioButton->isChecked() && !ui_->wineRecordRadioButton->isChecked());
    return WineSyscallHandlingMethod::kStopUnwinding;
  }
  if (ui_->wineRecordRadioButton->isChecked()) {
    ORBIT_CHECK(!ui_->wineNoneRadioButton->isChecked() && !ui_->wineStopRadioButton->isChecked());
    return WineSyscallHandlingMethod::kRecordUserStack;
  }
  ORBIT_UNREACHABLE();
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

void CaptureOptionsDialog::SetEnableAutoFrameTrack(bool enable_auto_frame_track) {
  ui_->autoFrameTrackCheckBox->setChecked(enable_auto_frame_track);
}

bool CaptureOptionsDialog::GetEnableAutoFrameTrack() const {
  return ui_->autoFrameTrackCheckBox->isChecked();
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
