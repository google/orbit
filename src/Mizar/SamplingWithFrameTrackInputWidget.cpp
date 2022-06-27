// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SamplingWithFrameTrackInputWidget.h"

#include <QWidget>
#include <memory>

#include "MizarData/MizarPairedData.h"
#include "ui_SamplingWithFrameTrackInputWidget.h"

namespace orbit_mizar {

template <typename PairedData>
SamplingWithFrameTrackInputWidgetTmpl<PairedData>::SamplingWithFrameTrackInputWidgetTmpl(
    QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackInputWidget>()) {
  ui_->setupUi(this);
}

template <typename PairedData>
void SamplingWithFrameTrackInputWidgetTmpl<PairedData>::Init(const PairedData* /*data*/,
                                                             const QString& name) {
  ui_->title_->setText(name);
}

template <typename PairedData>
SamplingWithFrameTrackInputWidgetTmpl<PairedData>::~SamplingWithFrameTrackInputWidgetTmpl() =
    default;

template class SamplingWithFrameTrackInputWidgetTmpl<orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar