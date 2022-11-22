// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_FILTER_PANEL_WIDGET_H_
#define ORBIT_QT_FILTER_PANEL_WIDGET_H_

#include <QFrame>
#include <QObject>
#include <QString>
#include <QWidget>

namespace Ui {
class FilterPanelWidget;  // IWYU pragma: keep
}

/*
 * A widget containing a track filter, a function filter, and a timer label. Will be added as a
 * widget action to the capture toolbar. (see FilterPanelWidgetAction.h for more details)
 */
class FilterPanelWidget : public QFrame {
  Q_OBJECT

 public:
  explicit FilterPanelWidget(QWidget* parent = nullptr);

  void SetFilterFunctionsText(const QString& text);
  void SetTimerLabelText(const QString& text);

  void ClearEdits();

 signals:
  void FilterTracksTextChanged(const QString& text);
  void FilterFunctionsTextChanged(const QString& text);

 private:
  Ui::FilterPanelWidget* ui;
};
#endif  // ORBIT_QT_FILTER_PANEL_WIDGET_H_