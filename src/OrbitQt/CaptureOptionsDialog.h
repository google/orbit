// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_
#define ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_

#include <QDialog>
#include <QObject>
#include <QString>
#include <QWidget>
#include <memory>

#include "OrbitBase/Logging.h"
#include "ui_CaptureOptionsDialog.h"

namespace orbit_qt {

class CaptureOptionsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit CaptureOptionsDialog(QWidget* parent = nullptr);

  void SetCollectThreadStates(bool collect_thread_state);
  [[nodiscard]] bool GetCollectThreadStates() const;

 private:
  std::unique_ptr<Ui::CaptureOptionsDialog> ui_;
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_CAPTURE_OPTIONS_DIALOG_H_
