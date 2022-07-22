// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"

#include <absl/strings/str_format.h>

#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QWidget>
#include <algorithm>
#include <iterator>
#include <limits>
#include <memory>

#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "ui_SamplingWithFrameTrackInputWidget.h"

namespace orbit_mizar_widgets {
SamplingWithFrameTrackInputWidgetBase::SamplingWithFrameTrackInputWidgetBase(QWidget* parent)
    : QWidget(parent), ui_(std::make_unique<Ui::SamplingWithFrameTrackInputWidget>()) {
  ui_->setupUi(this);
  ui_->start_ms_->setToolTip(
      QStringLiteral("Time in milliseconds since the capture started.\n"
                     "Only the portion after this time will be analyzed."));
  ui_->thread_list_->setToolTip(
      "Only the sampling data from the selected threads will be analyzed.\n"
      "Multiple selection is allowed.");
  QObject::connect(GetThreadList(), &QListWidget::itemSelectionChanged, this,
                   &SamplingWithFrameTrackInputWidgetBase::OnThreadSelectionChanged);
  QObject::connect(GetFrameTrackList(), qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &SamplingWithFrameTrackInputWidgetBase::OnFrameTrackSelectionChanged);
  QObject::connect(GetStartMs(), &QLineEdit::textChanged, this,
                   &SamplingWithFrameTrackInputWidgetBase::OnStartMsChanged);
}

QLabel* SamplingWithFrameTrackInputWidgetBase::GetTitle() const { return ui_->title_; }

QListWidget* SamplingWithFrameTrackInputWidgetBase::GetThreadList() const {
  return ui_->thread_list_;
}

QComboBox* SamplingWithFrameTrackInputWidgetBase::GetFrameTrackList() const {
  return ui_->frame_track_list_;
}

QLineEdit* SamplingWithFrameTrackInputWidgetBase::GetStartMs() const { return ui_->start_ms_; }

orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig
SamplingWithFrameTrackInputWidgetBase::MakeConfig() const {
  return orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig{
      selected_tids_, start_relative_time_ns_, std::numeric_limits<uint64_t>::max(),
      frame_track_id_};
}

void SamplingWithFrameTrackInputWidgetBase::OnThreadSelectionChanged() {
  selected_tids_.clear();
  const QList<QListWidgetItem*> selected_items = GetThreadList()->selectedItems();
  std::transform(std::begin(selected_items), std::end(selected_items),
                 std::inserter(selected_tids_, std::begin(selected_tids_)),
                 [](const QListWidgetItem* item) { return item->data(kTidRole).value<TID>(); });
}

void SamplingWithFrameTrackInputWidgetBase::OnFrameTrackSelectionChanged(int index) {
  frame_track_id_ = GetFrameTrackList()->itemData(index, kFrameTrackIdRole).value<FrameTrackId>();
}

void SamplingWithFrameTrackInputWidgetBase::OnStartMsChanged(const QString& time_ms) {
  if (time_ms.isEmpty()) {
    start_relative_time_ns_ = 0;
    return;
  }
  bool ok;
  constexpr uint64_t kNsInMs = 1'000'000;
  start_relative_time_ns_ = static_cast<uint64_t>(time_ms.toInt(&ok)) * kNsInMs;
  if (!ok) {
    start_relative_time_ns_ = std::numeric_limits<uint64_t>::max();
  }
}

SamplingWithFrameTrackInputWidgetBase::~SamplingWithFrameTrackInputWidgetBase() = default;

}  // namespace orbit_mizar_widgets