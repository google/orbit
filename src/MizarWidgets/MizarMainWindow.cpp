// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/MizarMainWindow.h"

#include <memory>

#include "ui_MizarMainWindow.h"

namespace orbit_mizar_widgets {

MizarMainWindow::MizarMainWindow() : ui_(std::make_unique<Ui::MainWindow>()) { ui_->setupUi(this); }

void MizarMainWindow::Init(const orbit_mizar_data::BaselineAndComparison* baseline_and_comparison) {
  ui_->sampling_with_frame_track_widget_->Init(baseline_and_comparison);
}

MizarMainWindow::~MizarMainWindow() = default;

}  // namespace orbit_mizar_widgets