// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>
#include <stdint.h>

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QWidget>
#include <Qt>
#include <limits>
#include <memory>
#include <string_view>
#include <vector>

#include "ClientData/ScopeInfo.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Sort.h"

namespace Ui {
class SamplingWithFrameTrackInputWidget;
}

Q_DECLARE_METATYPE(::orbit_mizar_base::TID);

namespace orbit_mizar_widgets {

class SamplingWithFrameTrackInputWidgetBase : public QWidget {
  Q_OBJECT
  using TID = ::orbit_mizar_base::TID;

 public:
  ~SamplingWithFrameTrackInputWidgetBase() override;

  [[nodiscard]] orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig MakeConfig() const;

 public slots:
  void OnThreadSelectionChanged();
  void OnFrameTrackSelectionChanged(int index);
  void OnStartMsChanged(const QString& time_ms);

 private:
  std::unique_ptr<Ui::SamplingWithFrameTrackInputWidget> ui_;

 protected:
  enum UserRoles { kTidRole = Qt::UserRole + 1, kScopeIdRole };

  explicit SamplingWithFrameTrackInputWidgetBase(QWidget* parent = nullptr);

  [[nodiscard]] QLabel* GetTitle() const;
  [[nodiscard]] QListWidget* GetThreadList() const;
  [[nodiscard]] QComboBox* GetFrameTrackList() const;
  [[nodiscard]] QLineEdit* GetStartMs() const;

 private:
  absl::flat_hash_set<TID> selected_tids_;
  uint64_t frame_track_scope_id_ = 0;

  // std::numeric_limits<uint64_t>::max() corresponds to malformed input
  uint64_t start_relative_time_ns_ = 0;
};

template <typename PairedData>
class SamplingWithFrameTrackInputWidgetTmpl : public SamplingWithFrameTrackInputWidgetBase {
  using TID = ::orbit_mizar_base::TID;

 public:
  SamplingWithFrameTrackInputWidgetTmpl() = delete;
  explicit SamplingWithFrameTrackInputWidgetTmpl(QWidget* parent)
      : SamplingWithFrameTrackInputWidgetBase(parent) {}
  ~SamplingWithFrameTrackInputWidgetTmpl() override = default;

  void Init(const PairedData& data, const QString& name) {
    InitTitle(name);
    InitThreadList(data);
    InitFrameTrackList(data);
    InitStartMs();
  }

 private:
  void InitTitle(const QString& name) { GetTitle()->setText(name); }

  void InitThreadList(const PairedData& data) {
    QListWidget* list = GetThreadList();
    list->setSelectionMode(QAbstractItemView::ExtendedSelection);

    const absl::flat_hash_map<TID, std::string>& tid_to_name = data.TidToNames();
    const absl::flat_hash_map<TID, uint64_t>& counts = data.TidToCallstackSampleCounts();

    std::vector<std::pair<TID, uint64_t>> counts_sorted(std::begin(counts), std::end(counts));

    orbit_base::sort(std::begin(counts_sorted), std::end(counts_sorted),
                     &std::pair<TID, uint64_t>::second, std::greater<>{});
    for (const auto& [tid, unused_count] : counts_sorted) {
      auto item = std::make_unique<QListWidgetItem>(
          QString::fromStdString(absl::StrFormat("[%u] %s", *tid, tid_to_name.at(tid))));
      item->setData(kTidRole, QVariant::fromValue(tid));
      list->addItem(item.release());
    }
  }

  void InitFrameTrackList(const PairedData& data) {
    const absl::flat_hash_map<uint64_t, orbit_client_data::ScopeInfo> scope_id_to_info =
        data.GetFrameTracks();
    std::vector<std::pair<uint64_t, orbit_client_data::ScopeInfo>> scope_id_to_info_sorted(
        std::begin(scope_id_to_info), std::end(scope_id_to_info));
    orbit_base::sort(std::begin(scope_id_to_info_sorted), std::end(scope_id_to_info_sorted),
                     [](const auto& a) { return a.second.GetName(); });

    for (size_t i = 0; i < scope_id_to_info_sorted.size(); ++i) {
      const auto& [scope_id, scope_info] = scope_id_to_info_sorted[i];
      GetFrameTrackList()->insertItem(i, MakeFrameTrackString(scope_info));
      GetFrameTrackList()->setItemData(i, QVariant::fromValue(scope_id), kScopeIdRole);
    }
    OnFrameTrackSelectionChanged(0);
  }

  void InitStartMs() {
    GetStartMs()->setValidator(new QIntValidator(0, std::numeric_limits<int>::max(), this));
    GetStartMs()->setText("0");
  }

  [[nodiscard]] static QString MakeFrameTrackString(
      const orbit_client_data::ScopeInfo& scope_info) {
    const std::string_view type_string =
        scope_info.GetType() == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction
            ? " D"
            : "MS";
    return QString::fromStdString(absl::StrFormat("[%s] %s", type_string, scope_info.GetName()));
  }
};

using SamplingWithFrameTrackInputWidget =
    SamplingWithFrameTrackInputWidgetTmpl<orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
