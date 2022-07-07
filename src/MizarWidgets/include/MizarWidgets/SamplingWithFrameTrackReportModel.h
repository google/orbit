// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_MODEL_H_
#define MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_MODEL_H_

#include <absl/container/flat_hash_map.h>

#include <QAbstractItemModel>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <iterator>
#include <vector>

#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_widgets {

template <typename Report, typename Counts, typename FrameTrackStats>
class SamplingWithFrameTrackReportModelTmpl : public QAbstractTableModel {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using SFID = ::orbit_mizar_base::SFID;

 public:
  enum class Column {
    kFunctionName,
    kBaselineExclusivePercent,
    kBaselineExclusiveTimePerFrame,
    kComparisonExclusivePercent,
    kComparisonExclusiveTimePerFrame,
    kPvalue,
    kIsSignificant,
    kSlowdownPercent,
    kSlowdownPerFrame
  };

  static constexpr int kColumnsCount = 9;

  explicit SamplingWithFrameTrackReportModelTmpl(Report report, QObject* parent = nullptr)
      : QAbstractTableModel(parent), report_(std::move(report)) {
    for (const auto& [sfid, unused_name] : report_.GetSfidToNames()) {
      if (*BaselineExclusiveCount(sfid) > 0 || *ComparisonExclusiveCount(sfid) > 0) {
        sfids_.push_back(sfid);
      }
    }
  }

  [[nodiscard]] int rowCount(const QModelIndex& /*parent*/) const override {
    return static_cast<int>(sfids_.size());
  };
  [[nodiscard]] int columnCount(const QModelIndex& /*parent*/) const override {
    return kColumnsCount;
  };
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
    if (role != Qt::DisplayRole) return {};
    return QString::fromStdString(MakeDisplayedString(index));
  }

  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role) const override {
    static const absl::flat_hash_map<Column, QString> kColumnNames = {
        {Column::kFunctionName, "Function"},
        {Column::kBaselineExclusivePercent, "Baseline, %"},
        {Column::kBaselineExclusiveTimePerFrame, "Baseline (per frame), us"},
        {Column::kComparisonExclusivePercent, "Comparison, %"},
        {Column::kComparisonExclusiveTimePerFrame, "Comparison (per frame), us"},
        {Column::kPvalue, "P-value"},
        {Column::kIsSignificant, "Significant?"},
        {Column::kSlowdownPercent, "Slowdown, %"},
        {Column::kSlowdownPerFrame, "Slowdown (per frame), us"}};

    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return {};
    }

    return kColumnNames.at(static_cast<Column>(section));
  }

 private:
  [[nodiscard]] std::string MakeDisplayedString(const QModelIndex& index) const {
    const SFID sfid = sfids_[index.row()];
    const auto column = static_cast<Column>(index.column());
    switch (column) {
      case Column::kFunctionName:
        return report_.GetSfidToNames().at(sfid);
      case Column::kBaselineExclusivePercent:
      case Column::kBaselineExclusiveTimePerFrame:
      case Column::kComparisonExclusivePercent:
      case Column::kComparisonExclusiveTimePerFrame:
        return absl::StrFormat("%.3f", (MakeNumericEntry(sfid, column)));
      case Column::kPvalue:
      case Column::kSlowdownPercent:
      case Column::kIsSignificant:
      case Column::kSlowdownPerFrame:
        return "Not Yet";
    }
  }
  [[nodiscard]] Baseline<double> BaselineExclusiveRate(SFID sfid) const {
    return LiftAndApply(&Counts::GetExclusiveRate, report_.GetBaselineSamplingCounts(),
                        Baseline<SFID>(sfid));
  }
  [[nodiscard]] Comparison<double> ComparisonExclusiveRate(SFID sfid) const {
    return LiftAndApply(&Counts::GetExclusiveRate, report_.GetComparisonSamplingCounts(),
                        Comparison<SFID>(sfid));
  }

  [[nodiscard]] Baseline<uint64_t> BaselineExclusiveCount(SFID sfid) const {
    return LiftAndApply(&Counts::GetExclusiveCount, report_.GetBaselineSamplingCounts(),
                        Baseline<SFID>(sfid));
  }
  [[nodiscard]] Comparison<uint64_t> ComparisonExclusiveCount(SFID sfid) const {
    return LiftAndApply(&Counts::GetExclusiveCount, report_.GetComparisonSamplingCounts(),
                        Comparison<SFID>(sfid));
  }

  [[nodiscard]] static double TimePerFrameUs(double rate,
                                             const FrameTrackStats& frame_track_stats) {
    constexpr uint kNsInUs = 1'000;
    return rate * frame_track_stats.ComputeAverageTimeNs() / kNsInUs;
  }

  [[nodiscard]] Baseline<double> BaselineExclusiveTimePerFrame(SFID sfid) const {
    return LiftAndApply(&TimePerFrameUs, BaselineExclusiveRate(sfid),
                        report_.GetBaselineFrameTrackStats());
  }
  [[nodiscard]] Comparison<double> ComparisonExclusiveTimePerFrame(SFID sfid) const {
    return LiftAndApply(&TimePerFrameUs, ComparisonExclusiveRate(sfid),
                        report_.GetComparisonFrameTrackStats());
  }

  [[nodiscard]] double MakeNumericEntry(SFID sfid, Column column) const {
    switch (column) {
      case Column::kBaselineExclusivePercent:
        return *BaselineExclusiveRate(sfid) * 100;
      case Column::kBaselineExclusiveTimePerFrame:
        return *BaselineExclusiveTimePerFrame(sfid);
      case Column::kComparisonExclusivePercent:
        return *ComparisonExclusiveRate(sfid) * 100;
      case Column::kComparisonExclusiveTimePerFrame:
        return *ComparisonExclusiveTimePerFrame(sfid);
      case Column::kPvalue:
      case Column::kSlowdownPercent:
      case Column::kSlowdownPerFrame:
        return 0;  // Not implemented yet
      default:
        ORBIT_UNREACHABLE();
    }
  }

  Report report_;
  std::vector<SFID> sfids_;
};

using SamplingWithFrameTrackReportModel =
    SamplingWithFrameTrackReportModelTmpl<orbit_mizar_data::SamplingWithFrameTrackComparisonReport,
                                          orbit_mizar_data::SamplingCounts,
                                          orbit_client_data::ScopeStats>;

}  // namespace orbit_mizar_widgets

#endif  // MIZAR_WIDGETS_SAMPLING_WITH_FRAME_TRACK_REPORT_MODEL_H_