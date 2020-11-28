// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_GL_WIDGET_H_
#define ORBIT_QT_ORBIT_GL_WIDGET_H_

// clang-format off
#include "OpenGl.h"
// clang-format on

#include <QOpenGLFunctions>
#include <QOpenGLWidget>

#include "GlCanvas.h"

class QOpenGLDebugMessage;
class QOpenGLDebugLogger;

class OrbitGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

 public:
  explicit OrbitGLWidget(QWidget* parent = nullptr);
  void Initialize(GlCanvas::CanvasType canvas_type, class OrbitMainWindow* a_MainWindow,
                  uint32_t font_size);
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

 signals:
  void checkFunctionHighlightChange();

 protected slots:
  void messageLogged(const QOpenGLDebugMessage& msg);
  void showContextMenu();
  void OnMenuClicked(int a_Index);

 private:
  std::unique_ptr<GlCanvas> gl_canvas_;
  QOpenGLDebugLogger* debug_logger_;
};

#endif  // ORBIT_QT_ORBIT_GL_WIDGET_H_
