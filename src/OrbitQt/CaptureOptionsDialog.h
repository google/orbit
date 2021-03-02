// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_
#define ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_

#include <QDialog>
#include <QObject>
#include <QString>
#include <QValidator>
#include <QWidget>
#include <memory>

#include "OrbitBase/Logging.h"
#include "ui_CaptureOptionsDialog.h"

namespace orbit_qt {

class UInt64Validator : public QValidator {
 public:
  explicit UInt64Validator(QObject* parent = nullptr) : QValidator(parent) {}
  QValidator::State validate(QString& input, int& /*pos*/) const override {
    if (input.isEmpty()) {
      return QValidator::State::Acceptable;
    }
    bool valid = false;
    input.toULongLong(&valid);
    if (valid) {
      return QValidator::State::Acceptable;
    }
    return QValidator::State::Invalid;
  }
};

class CaptureOptionsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit CaptureOptionsDialog(QWidget* parent = nullptr);

  void SetCollectThreadStates(bool collect_thread_state);
  void SetBulkCaptureEvents(bool bulk_capture_events);
  [[nodiscard]] bool GetCollectThreadStates() const;
  [[nodiscard]] bool GetBulkCaptureEvents() const;
  void SetLimitLocalMarkerDepthPerCommandBuffer(bool limit_local_marker_depth_per_command_buffer);
  [[nodiscard]] bool GetLimitLocalMarkerDepthPerCommandBuffer() const;
  void SetMaxLocalMarkerDepthPerCommandBuffer(uint64_t local_marker_depth_per_command_buffer);
  [[nodiscard]] uint64_t GetMaxLocalMarkerDepthPerCommandBuffer() const;

 public slots:
  void ResetLocalMarkerDepthLineEdit();
  void ShowSourcePathsMappingEditor();

 private:
  std::unique_ptr<Ui::CaptureOptionsDialog> ui_;
  UInt64Validator uint64_validator_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_
