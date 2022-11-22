// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/TrackConfigurationWidget.h"

#include <QHeaderView>
#include <QTableView>

#include "ui_TrackConfigurationWidget.h"

namespace orbit_qt {

TrackConfigurationWidget::TrackConfigurationWidget(QWidget* parent)
    : QWidget(parent),
      ui_(std::make_unique<Ui::TrackConfigurationWidget>()),
      track_type_item_model_(this) {
  ui_->setupUi(this);
  ui_->trackTypesTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
  ui_->trackTypesTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);
  ui_->trackTypesTable->setModel(&track_type_item_model_);
}

TrackConfigurationWidget::~TrackConfigurationWidget() = default;

void TrackConfigurationWidget::SetTrackManager(orbit_gl::TrackManager* track_manager) {
  track_type_item_model_.SetTrackManager(track_manager);
}

}  // namespace orbit_qt