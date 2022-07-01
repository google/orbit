// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"

#include <absl/strings/str_format.h>
#include <qcombobox.h>

#include <QComboBox>
#include <QListWidget>
#include <QObject>
#include <QWidget>
#include <algorithm>
#include <iterator>
#include <memory>

#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "ui_SamplingWithFrameTrackInputWidget.h"

namespace orbit_mizar_widgets {
SamplingWithFrameTrackInputWidgetBase::SamplingWithFrameTrackInputWidgetBase(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackInputWidget>()) {
  ui_->setupUi(this);
  QObject::connect(GetThreadList(), &QListWidget::itemSelectionChanged, this,
                   &SamplingWithFrameTrackInputWidgetBase::OnThreadSelectionChanged);
  QObject::connect(GetFrameTrackList(), qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &SamplingWithFrameTrackInputWidgetBase::OnFrameTrackSelectionChanged);
}

QLabel* SamplingWithFrameTrackInputWidgetBase::GetTitle() const { return ui_->title_; }

QListWidget* SamplingWithFrameTrackInputWidgetBase::GetThreadList() const {
  return ui_->thread_list_;
}

QComboBox* SamplingWithFrameTrackInputWidgetBase::GetFrameTrackList() const {
  return ui_->frame_track_list_;
}

orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig
SamplingWithFrameTrackInputWidgetBase::MakeConfig() const {
  return orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig{
      selected_tids_, 0 /*not implemented yet*/, 0 /*not implemented yet*/, frame_track_scope_id_};
}

void SamplingWithFrameTrackInputWidgetBase::OnThreadSelectionChanged() {
  selected_tids_.clear();
  const QList<QListWidgetItem*> selected_items = GetThreadList()->selectedItems();
  std::transform(std::begin(selected_items), std::end(selected_items),
                 std::inserter(selected_tids_, std::begin(selected_tids_)),
                 [](const QListWidgetItem* item) { return item->data(kTidRole).value<TID>(); });
}

void SamplingWithFrameTrackInputWidgetBase::OnFrameTrackSelectionChanged(int index) {
  frame_track_scope_id_ = GetFrameTrackList()->itemData(index, kScopeIdRole).value<uint64_t>();
}

SamplingWithFrameTrackInputWidgetBase::~SamplingWithFrameTrackInputWidgetBase() = default;

}  // namespace orbit_mizar_widgets