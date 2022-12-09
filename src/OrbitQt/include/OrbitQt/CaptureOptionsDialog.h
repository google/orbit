// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_
#define ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_

#include <stdint.h>

#include <QDialog>
#include <QObject>
#include <QString>
#include <QValidator>
#include <QWidget>
#include <limits>
#include <memory>

#include "ClientData/WineSyscallHandlingMethod.h"
#include "GrpcProtos/capture.pb.h"

namespace Ui {
class CaptureOptionsDialog;
}

namespace orbit_qt {

class UInt64Validator : public QValidator {
 public:
  explicit UInt64Validator(QObject* parent = nullptr) : QValidator(parent) {}
  explicit UInt64Validator(uint64_t minimum, uint64_t maximum, QObject* parent = nullptr)
      : QValidator(parent), minimum_(minimum), maximum_(maximum) {}
  QValidator::State validate(QString& input, int& /*pos*/) const override {
    if (input.isEmpty()) {
      return QValidator::State::Acceptable;
    }
    bool valid = false;
    uint64_t input_value = input.toULongLong(&valid);
    if (valid && input_value >= minimum_ && input_value <= maximum_) {
      return QValidator::State::Acceptable;
    }
    return QValidator::State::Invalid;
  }

 private:
  uint64_t minimum_ = 0;
  uint64_t maximum_ = std::numeric_limits<uint64_t>::max();
};

class CaptureOptionsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit CaptureOptionsDialog(QWidget* parent = nullptr);
  ~CaptureOptionsDialog() override;

  void SetEnableSampling(bool enable_sampling);
  [[nodiscard]] bool GetEnableSampling() const;
  void SetSamplingPeriodMs(double sampling_period_ms);
  [[nodiscard]] double GetSamplingPeriodMs() const;
  void SetUnwindingMethod(orbit_grpc_protos::CaptureOptions::UnwindingMethod unwinding_method);
  [[nodiscard]] orbit_grpc_protos::CaptureOptions::UnwindingMethod GetUnwindingMethod() const;
  void SetMaxCopyRawStackSize(uint16_t stack_dump_size);
  [[nodiscard]] uint16_t GetMaxCopyRawStackSize() const;
  void SetThreadStateChangeCallstackMaxCopyRawStackSize(uint16_t stack_dump_size);
  [[nodiscard]] uint16_t GetThreadStateChangeCallstackMaxCopyRawStackSize() const;
  void SetCollectSchedulerInfo(bool collect_scheduler_info);
  [[nodiscard]] bool GetCollectSchedulerInfo() const;
  void SetCollectThreadStates(bool collect_thread_state);
  [[nodiscard]] bool GetCollectThreadStates() const;
  void SetTraceGpuSubmissions(bool trace_gpu_submissions);
  [[nodiscard]] bool GetTraceGpuSubmissions() const;
  void SetEnableCallStackCollectionOnThreadStateChanges(bool check);
  [[nodiscard]] bool GetEnableCallStackCollectionOnThreadStateChanges() const;
  void SetEnableApi(bool enable_api);
  [[nodiscard]] bool GetEnableApi() const;
  void SetDynamicInstrumentationMethod(
      orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod method);
  [[nodiscard]] orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod
  GetDynamicInstrumentationMethod() const;
  void SetEnableIntrospection(bool enable_introspection);
  [[nodiscard]] bool GetEnableIntrospection() const;

  void SetWineSyscallHandlingMethod(orbit_client_data::WineSyscallHandlingMethod method);
  [[nodiscard]] orbit_client_data::WineSyscallHandlingMethod GetWineSyscallHandlingMethod() const;

  void SetLimitLocalMarkerDepthPerCommandBuffer(bool limit_local_marker_depth_per_command_buffer);
  [[nodiscard]] bool GetLimitLocalMarkerDepthPerCommandBuffer() const;
  void SetMaxLocalMarkerDepthPerCommandBuffer(uint64_t local_marker_depth_per_command_buffer);
  [[nodiscard]] uint64_t GetMaxLocalMarkerDepthPerCommandBuffer() const;

  void SetEnableAutoFrameTrack(bool enable_auto_frame_track);
  [[nodiscard]] bool GetEnableAutoFrameTrack() const;
  void SetCollectMemoryInfo(bool collect_memory_info);
  [[nodiscard]] bool GetCollectMemoryInfo() const;
  void SetMemorySamplingPeriodMs(uint64_t memory_sampling_period_ms);
  [[nodiscard]] uint64_t GetMemorySamplingPeriodMs() const;
  void SetMemoryWarningThresholdKb(uint64_t memory_warning_threshold_kb);
  [[nodiscard]] uint64_t GetMemoryWarningThresholdKb() const;

  static constexpr double kCallstackSamplingPeriodMsDefaultValue = 1.0;
  static constexpr orbit_grpc_protos::CaptureOptions::UnwindingMethod
      kCallstackUnwindingMethodDefaultValue = orbit_grpc_protos::CaptureOptions::kDwarf;
  static constexpr uint64_t kMemorySamplingPeriodMsDefaultValue = 10;
  static constexpr uint64_t kMemoryWarningThresholdKbDefaultValue = 1024 * 1024 * 8;  // 8Gb
  static constexpr orbit_grpc_protos::CaptureOptions::DynamicInstrumentationMethod
      kDynamicInstrumentationMethodDefaultValue =
          orbit_grpc_protos::CaptureOptions::kUserSpaceInstrumentation;
  static constexpr orbit_grpc_protos::CaptureOptions::ThreadStateChangeCallStackCollection
      kThreadStateChangeCallStackCollectionDefaultValue =
          orbit_grpc_protos::CaptureOptions::kNoThreadStateChangeCallStackCollection;
  static constexpr uint64_t kLocalMarkerDepthDefaultValue = 0;
  static constexpr orbit_client_data::WineSyscallHandlingMethod
      kWineSyscallHandlingMethodDefaultValue =
          orbit_client_data::WineSyscallHandlingMethod::kStopUnwinding;

  // Max to pass to perf_event_open without getting an error is (1u << 16u) - 8,
  // because the kernel stores this in a short and because of alignment reasons.
  // But the size the kernel actually returns is smaller and we leave some extra room (see
  // `PerfEventOpen.cpp`).
  static constexpr uint16_t kMaxCopyRawStackSizeDefaultValue = 512;
  static constexpr uint16_t kThreadStateChangeMaxCopyRawStackSizeDefaultValue = 256;

  // TODO(https://github.com/google/orbit/issues/4589): Connect slots via code and not via UI files,
  // and remove the "public slots" specifier
 public slots:  // NOLINT(readability-redundant-access-specifiers)
  void ResetLocalMarkerDepthLineEdit();
  void ResetMemorySamplingPeriodMsLineEditWhenEmpty();
  void ResetMemoryWarningThresholdKbLineEditWhenEmpty();

 private:
  std::unique_ptr<Ui::CaptureOptionsDialog> ui_;
  UInt64Validator uint64_validator_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_
