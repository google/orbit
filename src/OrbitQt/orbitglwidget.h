// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_GL_WIDGET_H_
#define ORBIT_QT_ORBIT_GL_WIDGET_H_

// clang-format off
#include "OpenGl.h" // IWYU pragma: keep
// clang-format on

#include <stdint.h>

// Disable "qopenglfunctions.h is not compatible with GLEW, GLEW defines will be undefined" warning
// to reduce spam in compilation output. This is a known quirk that doesn't cause any ill effect.
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-W#warnings"
#endif

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcpp"
#endif

#include <QOpenGLFunctions>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QOpenGLWidget>
#include <QString>
#include <QWheelEvent>
#include <QWidget>
#include <memory>

#include "GlCanvas.h"

class OrbitApp;
class OrbitMainWindow;
class QOpenGLDebugMessage;
class QOpenGLDebugLogger;

class OrbitGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

 public:
  explicit OrbitGLWidget(QWidget* parent = nullptr);
  void Initialize(GlCanvas::CanvasType canvas_type, OrbitMainWindow* main_window, OrbitApp* app);
  void Deinitialize(OrbitMainWindow* main_window);
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void TakeScreenShot();
  GlCanvas* GetCanvas() { return gl_canvas_.get(); }
  void PrintContextInformation();

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseDoubleClickEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void enterEvent(QEvent*) override;
  void leaveEvent(QEvent*) override;
  void keyPressEvent(QKeyEvent* event) override;
  void keyReleaseEvent(QKeyEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

 protected slots:
  void messageLogged(const QOpenGLDebugMessage& msg);
  void showContextMenu();
  void OnMenuClicked(int a_Index);

 private:
  std::unique_ptr<GlCanvas> gl_canvas_;
  QOpenGLDebugLogger* debug_logger_;
};

#endif  // ORBIT_QT_ORBIT_GL_WIDGET_H_
