// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_VIEWER_PLACE_HOLDER_WIDGET_H_
#define CODE_VIEWER_PLACE_HOLDER_WIDGET_H_

#include <QObject>
#include <QPaintEvent>
#include <QSize>
#include <QString>
#include <QWheelEvent>
#include <QWidget>

namespace orbit_code_viewer {

/*
  This placeholder widget's purpose is to consume space in a window
  and receive events for that space, which can behandled somewhere else.

  The size of that widget can be either determined by a layout; in this case
  the sizeHint is relevant, which can be set via `SetSizeHint`. If the widget
  is not added to a layout its size and position can be adjusted by
  `QWidget::setGeometry`.

  Whenever Qt requires this widget's area to be drawn, the widget will forward
  that point request by emitting the signal `PaintEventTriggered`. This signal
  has to be processed in the same thread (main thread usually) and can't be
  forwarded to an object associated with a different (background) thread.

  Usage example:

  auto place_holder = new PlaceHolderWidget();
  place_holder->setGeometry(QRect{10, 10, 100, 100});
  QObject::connect(place_holder, &PlaceHolderWidget::PaintEventTriggered,
                   [](QPaintEvent* ev) {
                     QPainter painter{place_holder};
                     painter.fillRect(ev->rect(), Qt::red);
                   });
*/
class PlaceHolderWidget : public QWidget {
  Q_OBJECT

 public:
  using QWidget::QWidget;

  [[nodiscard]] QSize sizeHint() const override { return size_hint_; }
  void SetSizeHint(const QSize& size) { size_hint_ = size; }

 private:
  QSize size_hint_;

  void paintEvent(QPaintEvent* event) override;
  void wheelEvent(QWheelEvent* ev) override;

 signals:
  void PaintEventTriggered(QPaintEvent* event);
  void WheelEventTriggered(QWheelEvent* event);
};

}  // namespace orbit_code_viewer

#endif  // CODE_VIEWER_PLACE_HOLDER_WIDGET_H_