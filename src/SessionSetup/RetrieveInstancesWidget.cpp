// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/RetrieveInstancesWidget.h"

#include <QLineEdit>
#include <memory>

#include "OrbitGgp/Client.h"
#include "ui_RetrieveInstancesWidget.h"

namespace orbit_session_setup {

RetrieveInstancesWidget::~RetrieveInstancesWidget() = default;

RetrieveInstancesWidget::RetrieveInstancesWidget(orbit_ggp::Client* ggp_client, QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::RetrieveInstancesWidget>()),
      ggp_client_(ggp_client),
      s_idle_(&state_machine_),
      s_loading_(&state_machine_),
      s_initial_loading_failed_(&state_machine_) {
  ui_->setupUi(this);

  QObject::connect(ui_->filterLineEdit, &QLineEdit::textChanged, this,
                   &RetrieveInstancesWidget::FilterTextChanged);
}

}  // namespace orbit_session_setup