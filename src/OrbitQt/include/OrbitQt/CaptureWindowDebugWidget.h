// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CAPTURE_WINDOW_DEBUG_WIDGET_H_
#define ORBIT_QT_CAPTURE_WINDOW_DEBUG_WIDGET_H_

#include <QObject>
#include <QString>
#include <QTimer>
#include <QWidget>
#include <memory>

#include "OrbitGl/CaptureWindowDebugInterface.h"
#include "OrbitGl/TimeGraphLayout.h"

namespace Ui {
class CaptureWindowDebugWidget;
}

namespace orbit_qt {

// The capture window debug widget has 2 purposes:
// 1. It integrates a TimeGraphLayoutWidget and exposes its interface by the GetTimeGraphLayout
// member function.
// 2. It takes a CaptureWindowDebugInterface and presents its debug information.
//
// It's used in Orbit's debug tab and targets Orbit developers.
class CaptureWindowDebugWidget : public QWidget {
  Q_OBJECT

 public:
  explicit CaptureWindowDebugWidget(QWidget* parent = nullptr);
  ~CaptureWindowDebugWidget() override;

  // Be aware the caller is responsible to keep `capture_window_debug_interface` alive until the
  // widget's lifetime ends or `ResetCaptureWindowDebugInterface` is called.
  void SetCaptureWindowDebugInterface(
      const orbit_gl::CaptureWindowDebugInterface* capture_window_debug_interface);
  void ResetCaptureWindowDebugInterface();

  [[nodiscard]] TimeGraphLayout* GetTimeGraphLayout() const;

 private:
  QTimer update_timer;
  void UpdateUiElements();

  std::unique_ptr<Ui::CaptureWindowDebugWidget> ui_;
  const orbit_gl::CaptureWindowDebugInterface* capture_window_debug_interface_ = nullptr;

 signals:
  void AnyLayoutPropertyChanged();
};

}  // namespace orbit_qt

#endif  // ORBIT_QT_CAPTURE_WINDOW_DEBUG_WIDGET_H_