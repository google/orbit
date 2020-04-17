//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

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
