// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_DEBUG_TAB_WIDGET_H_
#define ORBIT_QT_DEBUG_TAB_WIDGET_H_

#include <QObject>
#include <QString>
#include <QWidget>
#include <memory>

#include "OrbitGl/CaptureWindowDebugInterface.h"
#include "OrbitGl/TimeGraphLayout.h"

namespace Ui {
class DebugTabWidget;
}

namespace orbit_qt {

// The debug tab widget is the main widget of the debug tab. It itself has 2 nested tabs, one for
// the capture windows, and one for the introspection window.
class DebugTabWidget : public QWidget {
  Q_OBJECT

 public:
  explicit DebugTabWidget(QWidget* parent = nullptr);
  ~DebugTabWidget() override;

  [[nodiscard]] TimeGraphLayout* GetCaptureWindowTimeGraphLayout() const;
  [[nodiscard]] TimeGraphLayout* GetIntrospectionWindowTimeGraphLayout() const;

  void SetCaptureWindowDebugInterface(const orbit_gl::CaptureWindowDebugInterface* interface);
  void ResetCaptureWindowDebugInterface();

  void SetIntrospectionWindowDebugInterface(const orbit_gl::CaptureWindowDebugInterface* interface);
  void ResetIntrospectionWindowDebugInterface();

 private:
  std::unique_ptr<Ui::DebugTabWidget> ui_;

 signals:
  void AnyCaptureWindowPropertyChanged();
  void AnyIntrospectionWindowPropertyChanged();
};
}  // namespace orbit_qt

#endif  // ORBIT_QT_DEBUG_TAB_WIDGET_H_
