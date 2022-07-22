// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/functional/bind_front.h>
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
#include <type_traits>
#include <vector>

#include "ClientData/ScopeInfo.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/MizarPairedData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Sort.h"

namespace Ui {
class SamplingWithFrameTrackInputWidget;
}

Q_DECLARE_METATYPE(::orbit_mizar_base::TID);
Q_DECLARE_METATYPE(::orbit_mizar_data::FrameTrackId);

namespace orbit_mizar_widgets {

class SamplingWithFrameTrackInputWidgetBase : public QWidget {
  Q_OBJECT
  using TID = ::orbit_mizar_base::TID;
  using FrameTrackId = ::orbit_mizar_data::FrameTrackId;

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
  enum UserRoles { kTidRole = Qt::UserRole + 1, kFrameTrackIdRole };

  explicit SamplingWithFrameTrackInputWidgetBase(QWidget* parent = nullptr);

  [[nodiscard]] QLabel* GetTitle() const;
  [[nodiscard]] QListWidget* GetThreadList() const;
  [[nodiscard]] QComboBox* GetFrameTrackList() const;
  [[nodiscard]] QLineEdit* GetStartMs() const;

 private:
  absl::flat_hash_set<TID> selected_tids_;
  FrameTrackId frame_track_id_{};

  // std::numeric_limits<uint64_t>::max() corresponds to malformed input
  uint64_t start_relative_time_ns_ = 0;
};

template <typename PairedData>
class SamplingWithFrameTrackInputWidgetTmpl : public SamplingWithFrameTrackInputWidgetBase {
  using TID = ::orbit_mizar_base::TID;
  using FrameTrackId = ::orbit_mizar_data::FrameTrackId;
  using FrameTrackInfo = ::orbit_mizar_data::FrameTrackInfo;
  using PresentEvent = ::orbit_grpc_protos::PresentEvent;
  using ScopeInfo = ::orbit_client_data::ScopeInfo;

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

  [[nodiscard]] std::string MakeDisplayedName(const FrameTrackInfo& info) const {
    return Visit(&SamplingWithFrameTrackInputWidgetTmpl::MakeFrameTrackString,
                 &SamplingWithFrameTrackInputWidgetTmpl::PresentEventSourceName, info);
  }

  void InitFrameTrackList(const PairedData& data) {
    absl::flat_hash_map<FrameTrackId, FrameTrackInfo> id_to_infos = data.GetFrameTracks();
    std::vector<std::pair<FrameTrackId, std::string>> id_to_displayed_name;
    std::transform(std::begin(id_to_infos), std::end(id_to_infos),
                   std::back_inserter(id_to_displayed_name), [this](const auto& id_to_info) {
                     const auto [id, info] = id_to_info;
                     const std::string displayed_name = MakeDisplayedName(info);

                     return std::make_pair(id, displayed_name);
                   });

    orbit_base::sort(std::begin(id_to_displayed_name), std::end(id_to_displayed_name),
                     &std::pair<FrameTrackId, std::string>::second);

    for (size_t i = 0; i < id_to_displayed_name.size(); ++i) {
      const auto& [id, displayed_name] = id_to_displayed_name[i];
      GetFrameTrackList()->insertItem(i, QString::fromStdString(displayed_name));
      GetFrameTrackList()->setItemData(i, QVariant::fromValue(id), kFrameTrackIdRole);
    }
    OnFrameTrackSelectionChanged(0);
  }

  void InitStartMs() {
    GetStartMs()->setValidator(new QIntValidator(0, std::numeric_limits<int>::max(), this));
    GetStartMs()->setText("0");
  }

  [[nodiscard]] static std::string MakeFrameTrackString(
      const orbit_client_data::ScopeInfo& scope_info) {
    const std::string_view type_string =
        scope_info.GetType() == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction
            ? "  D"
            : " MS";
    return absl::StrFormat("[%s] %s", type_string, scope_info.GetName());
  }

  [[nodiscard]] static std::string PresentEventSourceName(PresentEvent::Source source) {
    switch (source) {
      case PresentEvent::kD3d9:
        return "[ETW] D3d9";
      case PresentEvent::kDxgi:
        return "[ETW] Dxgi";
      default:
        ORBIT_UNREACHABLE();
    }
  }
};

using SamplingWithFrameTrackInputWidget =
    SamplingWithFrameTrackInputWidgetTmpl<orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_INPUT_WIDGET_H_
