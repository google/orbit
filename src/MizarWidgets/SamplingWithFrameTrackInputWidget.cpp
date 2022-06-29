// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"

#include <absl/strings/str_format.h>

#include <QListWidget>
#include <QObject>
#include <QWidget>
#include <algorithm>
#include <iterator>
#include <memory>

#include "ui_SamplingWithFrameTrackInputWidget.h"

namespace orbit_mizar_widgets {
SamplingWithFrameTrackInputWidgetBase::SamplingWithFrameTrackInputWidgetBase(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackInputWidget>()) {
  ui_->setupUi(this);
  QObject::connect(GetThreadList(), &QListWidget::itemSelectionChanged, this,
                   &SamplingWithFrameTrackInputWidgetBase::OnThreadSelectionChanged);
}

QLabel* SamplingWithFrameTrackInputWidgetBase::GetTitle() const { return ui_->title_; }

QListWidget* SamplingWithFrameTrackInputWidgetBase::GetThreadList() const {
  return ui_->thread_list_;
}

void SamplingWithFrameTrackInputWidgetBase::OnThreadSelectionChanged() {
  selected_tids_.clear();
  const QList<QListWidgetItem*> selected_items = GetThreadList()->selectedItems();
  std::transform(
      std::begin(selected_items), std::end(selected_items),
      std::inserter(selected_tids_, std::begin(selected_tids_)),
      [this](const QListWidgetItem* item) { return tid_list_widget_items_to_tids_.at(item); });
}

SamplingWithFrameTrackInputWidgetBase::~SamplingWithFrameTrackInputWidgetBase() {
  disconnect(GetThreadList(), nullptr, nullptr, nullptr);
}

}  // namespace orbit_mizar_widgets