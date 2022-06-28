// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingWithFrameTrackInputWidget.h"

#include <QWidget>
#include <memory>

#include "MizarData/MizarPairedData.h"
#include "ui_SamplingWithFrameTrackInputWidget.h"

namespace orbit_mizar {
SamplingWithFrameTrackInputWidget::SamplingWithFrameTrackInputWidget(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackInputWidget>()) {
  ui_->setupUi(this);
}

template <typename PairedData>
void SamplingWithFrameTrackInputWidget::Init(const PairedData* /*data*/, const QString& name) {
  ui_->title_->setText(name);
}

SamplingWithFrameTrackInputWidget::~SamplingWithFrameTrackInputWidget() = default;

template void SamplingWithFrameTrackInputWidget::Init<>(
    const orbit_mizar_data::MizarPairedData* data, const QString& name);

}  // namespace orbit_mizar