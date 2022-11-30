// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_MODELS_FRAME_TRACK_LIST_MODEL_H_
#define MIZAR_MODELS_FRAME_TRACK_LIST_MODEL_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>

#include <QAbstractListModel>
#include <QString>
#include <Qt>
#include <string>

#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/MizarPairedData.h"
#include "OrbitBase/Overloaded.h"
#include "OrbitBase/Sort.h"

Q_DECLARE_METATYPE(::orbit_mizar_data::FrameTrackId);

namespace orbit_mizar_models {

constexpr int kFrameTrackIdRole = Qt::UserRole + 1;

template <typename PairedData>
class FrameTrackListModelTmpl : public QAbstractListModel {
  using FrameTrackId = ::orbit_mizar_data::FrameTrackId;
  using FrameTrackInfo = ::orbit_mizar_data::FrameTrackInfo;
  using RelativeTimeNs = ::orbit_mizar_base::RelativeTimeNs;
  using TID = ::orbit_mizar_base::TID;
  using PresentEvent = ::orbit_grpc_protos::PresentEvent;

 public:
  FrameTrackListModelTmpl(const PairedData* data, const absl::flat_hash_set<TID>* selected_tids,
                          const RelativeTimeNs* start_timestamp, QObject* parent = nullptr)
      : QAbstractListModel(parent),
        data_(data),
        selected_tids_(selected_tids),
        start_timestamp_(start_timestamp),
        frame_tracks_(MakeDisplayedNames(data)) {}

  [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
    return parent.isValid() ? 0 : frame_tracks_.size();
  }

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
    if (index.model() != this) return {};
    const auto& [id, name] = frame_tracks_[index.row()];
    switch (role) {
      case Qt::DisplayRole:
        return QString::fromStdString(name);
      case Qt::ToolTipRole:
        return MakeTooltip(id, name);
      case kFrameTrackIdRole:
        return QVariant::fromValue(id);
      default:
        return {};
    }
  }

 private:
  struct FrameTrack {
    FrameTrackId id;
    std::string displayed_name;
  };

  [[nodiscard]] static std::vector<FrameTrack> MakeDisplayedNames(const PairedData* data) {
    absl::flat_hash_map<FrameTrackId, FrameTrackInfo> id_to_infos = data->GetFrameTracks();
    std::vector<FrameTrack> frame_tracks;
    std::transform(std::begin(id_to_infos), std::end(id_to_infos), std::back_inserter(frame_tracks),
                   [](const auto& id_to_info) {
                     const auto [id, info] = id_to_info;
                     return FrameTrack{id, MakeDisplayedName(info)};
                   });

    orbit_base::sort(std::begin(frame_tracks), std::end(frame_tracks), &FrameTrack::displayed_name);
    return frame_tracks;
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

  [[nodiscard]] static std::string MakeDisplayedName(const FrameTrackInfo& info) {
    return std::visit(orbit_base::overloaded{&MakeFrameTrackString, &PresentEventSourceName},
                      *info);
  }

  [[nodiscard]] QString MakeTooltip(FrameTrackId id, std::string_view name) const {
    const auto& [wall_clock_time, active_invocation_time] =
        data_->WallClockAndActiveInvocationTimeStats(
            *selected_tids_, id, *start_timestamp_,
            orbit_mizar_base::RelativeTimeNs(std::numeric_limits<uint64_t>::max()));

    constexpr double kNsInMs = 1e6;
    const double average_wall_clock_ms = wall_clock_time.ComputeAverageTimeNs() / kNsInMs;
    const double average_active_ms = active_invocation_time.ComputeAverageTimeNs() / kNsInMs;
    ORBIT_CHECK(wall_clock_time.count() == active_invocation_time.count());
    return QString::fromStdString(
        absl::StrFormat("The frame track \"%s\" has %u frames\n"
                        "with average wall-clock time of %.3f ms,\n"
                        "with average CPU time across selected threads of %.3f ms.",
                        name, wall_clock_time.count(), average_wall_clock_ms, average_active_ms));
  }

  const PairedData* data_;
  const absl::flat_hash_set<TID>* selected_tids_{};
  const RelativeTimeNs* start_timestamp_{};
  std::vector<FrameTrack> frame_tracks_;
};

// The instantiation is supposed to be used in production
using FrameTrackListModel = FrameTrackListModelTmpl<orbit_mizar_data::MizarPairedData>;

}  // namespace orbit_mizar_models

#endif  // MIZAR_MODELS_FRAME_TRACK_LIST_MODEL_H_
