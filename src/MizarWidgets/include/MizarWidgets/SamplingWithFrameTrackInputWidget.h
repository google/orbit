// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <QLabel>
#include <QListWidget>
#include <QObject>
#include <QWidget>
#include <memory>
#include <string_view>
#include <vector>

#include "MizarData/MizarPairedData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"

namespace Ui {
class SamplingWithFrameTrackInputWidget;
}

namespace orbit_mizar_widgets {

class SamplingWithFrameTrackInputWidgetBase : public QWidget {
  Q_OBJECT
 public:
  ~SamplingWithFrameTrackInputWidgetBase() override;

  [[nodiscard]] orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig MakeConfig() const;

 public slots:
  void OnThreadSelectionChanged();

 private:
  std::unique_ptr<Ui::SamplingWithFrameTrackInputWidget> ui_;

 protected:
  explicit SamplingWithFrameTrackInputWidgetBase(QWidget* parent);

  [[nodiscard]] QLabel* GetTitle() const;
  [[nodiscard]] QListWidget* GetThreadList() const;

  absl::node_hash_map<std::unique_ptr<QListWidgetItem>, uint32_t> tid_list_widget_items_to_tids_;

 private:
  absl::flat_hash_set<uint32_t> selected_tids_;
};

template <typename PairedData>
class SamplingWithFrameTrackInputWidgetTmpl : public SamplingWithFrameTrackInputWidgetBase {
 public:
  SamplingWithFrameTrackInputWidgetTmpl() = delete;
  explicit SamplingWithFrameTrackInputWidgetTmpl(QWidget* parent)
      : SamplingWithFrameTrackInputWidgetBase(parent) {}
  ~SamplingWithFrameTrackInputWidgetTmpl() override = default;

  void Init(const PairedData& data, const QString& name) {
    InitTitle(name);
    InitThreadList(data);
  }

 private:
  void InitTitle(const QString& name) { GetTitle()->setText(name); }

  void InitThreadList(const PairedData& data) {
    QListWidget* list = GetThreadList();
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    absl::flat_hash_map<uint32_t, std::string> tid_to_name = data.TidToNames();
    absl::flat_hash_map<uint32_t, uint64_t> counts = data.TidToCallstackSampleCounts();

    std::vector<std::pair<uint32_t, uint64_t>> counts_sorted(std::begin(counts), std::end(counts));

    std::sort(std::begin(counts_sorted), std::end(counts_sorted),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    for (const auto& [tid, unused_count] : counts_sorted) {
      auto item = std::make_unique<QListWidgetItem>(
          QString::fromStdString(absl::StrFormat("[%u] %s", tid, tid_to_name.at(tid))));
      list->addItem(item.get());
      tid_list_widget_items_to_tids_.try_emplace(std::move(item), tid);
    }
  }
};

using SamplingWithFrameTrackInputWidget =
    SamplingWithFrameTrackInputWidgetTmpl<orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
