// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_FILTER_PANEL_WIDGET_ACTION_H_
#define ORBIT_QT_FILTER_PANEL_WIDGET_ACTION_H_

#include <QFrame>
#include <QObject>
#include <QString>
#include <QWidget>
#include <QWidgetAction>

#include "OrbitQt/FilterPanelWidget.h"

/*
 * Widget action used to insert the custom widget, i.e., FilterPanelWidget, into capture toolbar.
 *
 * As the capture toolbar is not a child of a QMainWindow, it loses the ability to populate the
 * extension pop up with widgets added to the toolbar using addWidget(). To insert custom widgets,
 * we need widget actions created by inheriting QWidgetAction and implementing
 * QWidgetAction::createWidget() method.
 */
class FilterPanelWidgetAction : public QWidgetAction {
  Q_OBJECT

 public:
  explicit FilterPanelWidgetAction(QWidget* parent);
  QWidget* createWidget(QWidget* parent);

 signals:
  void FilterFunctionsTextChanged(const QString& text);
  void FilterTracksTextChanged(const QString& text);
  void SetTimerLabelText(const QString& text);
  void SetFilterFunctionsText(const QString& text);
  void ClearEdits();

 private:
  FilterPanelWidget* filter_panel_ = nullptr;
};
#endif  // ORBIT_QT_FILTER_PANEL_WIDGET_ACTION_H_