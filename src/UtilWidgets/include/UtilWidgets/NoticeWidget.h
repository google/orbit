// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UTIL_WIDGETS_NOTICE_WIDGET_H_
#define UTIL_WIDGETS_NOTICE_WIDGET_H_

#include <QColor>
#include <QObject>
#include <QPaintEvent>
#include <QString>
#include <QWidget>
#include <memory>
#include <string>
#include <string_view>

namespace Ui {
class NoticeWidget;
}

namespace orbit_util_widgets {

class NoticeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit NoticeWidget(QWidget* parent = nullptr);
  ~NoticeWidget() override;

  void Initialize(std::string_view label_text, std::string_view button_text, const QColor& color);
  void InitializeAsInspection();

 signals:
  void ButtonClicked();

 protected:
  void paintEvent(QPaintEvent* event) override;

 private:
  std::unique_ptr<Ui::NoticeWidget> ui_;
};

}  // namespace orbit_util_widgets

#endif  // UTIL_WIDGETS_NOTICE_WIDGET_H_