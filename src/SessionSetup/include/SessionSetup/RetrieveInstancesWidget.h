// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_RETRIEVE_INSTANCES_WIDGET_H_
#define SESSION_SETUP_RETRIEVE_INSTANCES_WIDGET_H_

#include <QObject>
#include <QState>
#include <QStateMachine>
#include <QWidget>
#include <memory>

#include "OrbitGgp/Client.h"

namespace Ui {
class RetrieveInstancesWidget;
}

namespace orbit_session_setup {

class RetrieveInstancesWidget : public QWidget {
  Q_OBJECT

 public:
  explicit RetrieveInstancesWidget(orbit_ggp::Client* ggp_client, QWidget* parent = nullptr);
  ~RetrieveInstancesWidget() override;

 signals:
  void FilterTextChanged(const QString& text);

 private:
  std::unique_ptr<Ui::RetrieveInstancesWidget> ui_;
  orbit_ggp::Client* ggp_client_;
  QStateMachine state_machine_;
  QState s_idle_;
  QState s_loading_;
  QState s_initial_loading_failed_;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_RETRIEVE_INSTANCES_WIDGET_H_