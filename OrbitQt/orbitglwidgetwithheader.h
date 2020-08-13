// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_ORBIT_GL_WIDGET_WITH_HEADER_H_
#define ORBIT_QT_ORBIT_GL_WIDGET_WITH_HEADER_H_

#include <QWidget>

namespace Ui {
class OrbitGlWidgetWithHeader;
}

class OrbitGlWidgetWithHeader : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitGlWidgetWithHeader(QWidget* parent = nullptr);
  ~OrbitGlWidgetWithHeader() override;

  class OrbitTreeView* GetTreeView();
  class OrbitGLWidget* GetGLWidget();

 private:
  Ui::OrbitGlWidgetWithHeader* ui;
};

#endif  // ORBIT_QT_ORBIT_GL_WIDGET_WITH_HEADER_H_
