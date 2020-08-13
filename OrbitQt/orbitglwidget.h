// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_GL_WIDGET_H_
#define ORBIT_QT_ORBIT_GL_WIDGET_H_

#include <QOpenGLFunctions>
#include <QOpenGLWidget>

#include "../OrbitGl/GlPanel.h"

class QOpenGLDebugMessage;
class QOpenGLDebugLogger;

class OrbitGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
  Q_OBJECT

 public:
  explicit OrbitGLWidget(QWidget* parent = nullptr);
  void Initialize(GlPanel::Type a_Type, class OrbitMainWindow* a_MainWindow);
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void TakeScreenShot();
  GlPanel* GetPanel() { return m_OrbitPanel.get(); }
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
  std::unique_ptr<GlPanel> m_OrbitPanel;
  QOpenGLDebugLogger* m_DebugLogger;
};

#endif  // ORBIT_QT_ORBIT_GL_WIDGET_H_
