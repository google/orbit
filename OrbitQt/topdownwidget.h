// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TOP_DOWN_WIDGET_H_
#define ORBIT_QT_TOP_DOWN_WIDGET_H_

#include <QSortFilterProxyModel>
#include <memory>

#include "TopDownView.h"
#include "ui_topdownwidget.h"

class TopDownWidget : public QWidget {
  Q_OBJECT

 public:
  explicit TopDownWidget(QWidget* parent = nullptr)
      : QWidget{parent}, ui_{std::make_unique<Ui::TopDownWidget>()} {
    ui_->setupUi(this);
  }

  void SetTopDownView(std::unique_ptr<TopDownView> top_down_view);

 private:
  std::unique_ptr<Ui::TopDownWidget> ui_;
};

#endif  // ORBIT_QT_TOP_DOWN_WIDGET_H_
