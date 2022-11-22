// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_GL_WIDGET_H_
#define ORBIT_QT_ORBIT_GL_WIDGET_H_

#include <stdint.h>

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QString>
#include <QTimer>
#include <QWheelEvent>
#include <QWidget>
#include <memory>

#include "OrbitGl/GlCanvas.h"
#include "OrbitGl/OrbitApp.h"
#include "OrbitGl/TimeGraphLayout.h"

class OrbitGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

 public:
  explicit OrbitGLWidget(QWidget* parent = nullptr);
  void Initialize(std::unique_ptr<GlCanvas> gl_canvas);
  void Deinitialize();
  [[nodiscard]] GlCanvas* GetCanvas() { return gl_canvas_.get(); }
  [[nodiscard]] const GlCanvas* GetCanvas() const { return gl_canvas_.get(); }

 private:
  void PrintContextInformation();
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void enterEvent(QEvent* event) override;
  void leaveEvent(QEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

  std::unique_ptr<GlCanvas> gl_canvas_;
  QTimer update_timer_;
};

#endif  // ORBIT_QT_ORBIT_GL_WIDGET_H_
