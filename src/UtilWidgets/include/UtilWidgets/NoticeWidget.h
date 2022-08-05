// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UTIL_WIDGETS_NOTICE_WIDGET_H_
#define UTIL_WIDGETS_NOTICE_WIDGET_H_

#include <QWidget>

namespace Ui {
class NoticeWidget;
}

namespace orbit_util_widgets {

class NoticeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit NoticeWidget(QWidget* parent = nullptr);
  ~NoticeWidget() override;

  void Initialize(const std::string& label_text, const std::string& button_text);
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