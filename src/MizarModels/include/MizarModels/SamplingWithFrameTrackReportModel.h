// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_MODELS_SAMPLING_WITH_FRAME_TRACK_REPORT_MODEL_H_
#define MIZAR_MODELS_SAMPLING_WITH_FRAME_TRACK_REPORT_MODEL_H_

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <stdint.h>

#include <QAbstractItemModel>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <iterator>
#include <vector>

#include "MizarBase/Titles.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_models {

// The class implements the model for the instance of `QTableView` owned by
// `SamplingWithFrameTrackOutputWidget`. It represents the results of comparison based on sampling
// data with frame track.
template <typename Report, typename Counts, typename FrameTrackStats>
class SamplingWithFrameTrackReportModelTmpl : public QAbstractTableModel {
  template <typename T>
  using Baseline = ::orbit_mizar_base::Baseline<T>;
  template <typename T>
  using Comparison = ::orbit_mizar_base::Comparison<T>;
  using SFID = ::orbit_mizar_base::SampledFunctionId;
  using BaselineAndComparisonFunctionSymbols =
      orbit_mizar_base::BaselineAndComparisonFunctionSymbols;

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
    kPercentOfSlowdown
  };

  enum class FunctionNameToShow { kBaseline, kComparison };

  static constexpr int kColumnsCount = 9;

  explicit SamplingWithFrameTrackReportModelTmpl(Report report,
                                                 bool is_multiplicity_correction_enabled,
                                                 double significance_level,
                                                 FunctionNameToShow function_name_to_show,
                                                 QObject* parent = nullptr)
      : QAbstractTableModel(parent),
        report_(std::move(report)),
        is_multiplicity_correction_enabled_(is_multiplicity_correction_enabled),
        significance_level_(significance_level),
        function_name_to_show_(function_name_to_show) {
    for (const auto& [sfid, unused_symbol] : report_.GetSfidToSymbols()) {
      if (*BaselineExclusiveCount(sfid) > 0 || *ComparisonExclusiveCount(sfid) > 0) {
        sfids_.push_back(sfid);
      }
    }
  }

  void SetMultiplicityCorrectionEnabled(bool is_enabled) {
    is_multiplicity_correction_enabled_ = is_enabled;
    EmitDataChanged(Column::kPvalue);
  }

  void SetSignificanceLevel(double significance_level) {
    significance_level_ = significance_level;
    EmitDataChanged(Column::kIsSignificant);
  }

  void SetFunctionNameToShow(FunctionNameToShow function_name_to_show) {
    if (function_name_to_show_ != function_name_to_show) {
      function_name_to_show_ = function_name_to_show;
      EmitDataChanged(Column::kFunctionName);
    }
  }

  [[nodiscard]] int rowCount(const QModelIndex& parent) const override {
    return parent.isValid() ? 0 : static_cast<int>(sfids_.size());
  };
  [[nodiscard]] int columnCount(const QModelIndex& parent) const override {
    return parent.isValid() ? 0 : kColumnsCount;
  };
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
    if (index.model() != this) return {};
    switch (role) {
      case Qt::DisplayRole:
        return QString::fromStdString(MakeDisplayedString(index));
      case Qt::EditRole:
        return MakeSortValue(index);
      case Qt::ToolTipRole:
        return MakeTooltip(index);
      default:
        return {};
    }
  }

  [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation,
                                    int role = Qt::DisplayRole) const override {
    static const absl::flat_hash_map<Column, QString> kColumnNames = {
        {Column::kFunctionName, "Function"},
        {Column::kBaselineExclusivePercent, "Baseline, %"},
        {Column::kBaselineExclusiveTimePerFrame, "Baseline (per frame), us"},
        {Column::kComparisonExclusivePercent, "Comparison, %"},
        {Column::kComparisonExclusiveTimePerFrame, "Comparison (per frame), us"},
        {Column::kPvalue, "P-value"},
        {Column::kIsSignificant, "Significant?"},
        {Column::kSlowdownPercent, "Slowdown, %"},
        {Column::kPercentOfSlowdown, "% of frametime slowdown"}};

    if (orientation == Qt::Vertical || role != Qt::DisplayRole) {
      return {};
    }

    return kColumnNames.at(static_cast<Column>(section));
  }

 private:
  struct Index {
    SFID sfid;
    Column column;
  };

  void EmitDataChanged(Column column) {
    const int column_int = static_cast<int>(column);
    emit dataChanged(index(0, column_int), index(rowCount({}) - 1, column_int));
  }

  [[nodiscard]] Index MakeIndex(const QModelIndex& index) const {
    return {sfids_[index.row()], static_cast<Column>(index.column())};
  }

  [[nodiscard]] bool IsNumeric(const Index& index, int role) const {
    switch (index.column) {
      case Column::kBaselineExclusivePercent:
      case Column::kBaselineExclusiveTimePerFrame:
      case Column::kComparisonExclusivePercent:
      case Column::kComparisonExclusiveTimePerFrame:
      case Column::kPvalue:
      case Column::kSlowdownPercent:
      case Column::kPercentOfSlowdown:
        return true;
      case Column::kIsSignificant:
        return role != Qt::DisplayRole;
      default:
        return false;
    }
  }

  [[nodiscard]] static QString MakeTooltipForSamplingColumns(const QString& title,
                                                             const std::string* function_name,
                                                             uint64_t count, double rate) {
    return QString::fromStdString(
        absl::StrFormat("The function \"%s\"\n"
                        "was encountered %u times (inclusive count) "
                        "in the %s capture.\n"
                        "This makes up for %.3f%% of the samples.",
                        *function_name, count, title.toStdString(), rate * 100));
  }

  [[nodiscard]] static QString MakeTooltipForTimePerFrameColumns(const QString& title,
                                                                 const std::string* function_name,
                                                                 double time) {
    return QString::fromStdString(
        absl::StrFormat("In the %s capture %.3f microseconds of CPU\n"
                        "time were spent to compute the\n"
                        "function \"%s\".\n"
                        "Note. This time also includes the time spent to compute\n"
                        "the functions it called that are not present in both captures.",
                        title.toStdString(), time, *function_name));
  }

  [[nodiscard]] static QString MakeTooltipForSlowdownColumn(const std::string* function_name,
                                                            double slowdown_percent) {
    return QString::fromStdString(
        absl::StrFormat("The function \"%s\" is  %.3f%%\n"
                        "slower in %s capture that it was in %s capture.\n"
                        "Negative percentage represent a speed-up.",
                        *function_name, slowdown_percent, *orbit_mizar_base::kComparisonTitle,
                        *orbit_mizar_base::kBaselineTitle));
  }

  [[nodiscard]] static QString MakeTooltipForPercentOfSlowdownColumn(
      const std::string* function_name, double percent_of_slowdown) {
    return QString::fromStdString(
        absl::StrFormat("The slowdown of function \"%s\" constitutes  %.3f%%\n"
                        "of the total frametime slowdown in %s capture compared to %s capture.\n"
                        "Negative percentage represent a speed-up.",
                        *function_name, percent_of_slowdown, *orbit_mizar_base::kBaselineTitle,
                        *orbit_mizar_base::kComparisonTitle));
  }

  [[nodiscard]] QVariant MakeTooltip(const QModelIndex& model_index) const {
    const auto& [sfid, column] = MakeIndex(model_index);
    const std::string* function_name = &GetFunctionName(sfid);

    switch (column) {
      case Column::kFunctionName:
        return QString::fromStdString(*function_name);
      case Column::kBaselineExclusivePercent:
        return *LiftAndApply(&MakeTooltipForSamplingColumns, orbit_mizar_base::QBaselineTitle(),
                             Baseline<const std::string*>(function_name),
                             BaselineExclusiveCount(sfid), BaselineExclusiveRate(sfid));
      case Column::kComparisonExclusivePercent:
        return *LiftAndApply(&MakeTooltipForSamplingColumns, orbit_mizar_base::QComparisonTitle(),
                             Comparison<const std::string*>(function_name),
                             ComparisonExclusiveCount(sfid), ComparisonExclusiveRate(sfid));
      case Column::kBaselineExclusiveTimePerFrame:
        return *LiftAndApply(&MakeTooltipForTimePerFrameColumns, orbit_mizar_base::QBaselineTitle(),
                             Baseline<const std::string*>(function_name),
                             BaselineExclusiveTimePerFrameUs(sfid));
      case Column::kComparisonExclusiveTimePerFrame:
        return *LiftAndApply(
            &MakeTooltipForTimePerFrameColumns, orbit_mizar_base::QComparisonTitle(),
            Comparison<const std::string*>(function_name), ComparisonExclusiveTimePerFrameUs(sfid));
      case Column::kPvalue:
        return "P-value is a term from statistics.\n"
               "The lower it is, the less we \"believe\"\n"
               "that the function runtime does not differ\n"
               "between the captures.";
      case Column::kIsSignificant:
        return "The difference is deemed significant if\n"
               "p-value is less then false-alarm probability";
      case Column::kSlowdownPercent:
        return MakeTooltipForSlowdownColumn(function_name, SlowdownPercent(sfid));
      case Column::kPercentOfSlowdown:
        return MakeTooltipForPercentOfSlowdownColumn(function_name, PercentOfFrameSlowdown(sfid));
      default:
        return {};
    }
  }

  [[nodiscard]] QVariant MakeSortValue(const QModelIndex& model_index) const {
    const Index index = MakeIndex(model_index);

    if (IsNumeric(index, Qt::EditRole)) return MakeNumericEntry(index);
    return QString::fromStdString(MakeStringEntry(index));
  }

  [[nodiscard]] std::string MakeDisplayedString(const QModelIndex& model_index) const {
    const Index index = MakeIndex(model_index);

    if (IsNumeric(index, Qt::DisplayRole)) return absl::StrFormat("%.3f", MakeNumericEntry(index));
    return MakeStringEntry(index);
  }

  [[nodiscard]] std::string MakeStringEntry(const Index& index) const {
    const auto [sfid, column] = index;
    switch (column) {
      case Column::kFunctionName:
        return GetFunctionName(sfid);
      case Column::kIsSignificant:
        return GetPvalue(sfid) < significance_level_ ? "Yes" : "No";
      default:
        ORBIT_UNREACHABLE();
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

  inline static constexpr uint64_t kNsInUs = 1'000;

  [[nodiscard]] static double TimePerFrameUs(double rate,
                                             const FrameTrackStats& frame_track_stats) {
    return rate * frame_track_stats.ComputeAverageTimeNs() / kNsInUs;
  }

  [[nodiscard]] static double AverageFrameTime(const FrameTrackStats& stats) {
    return static_cast<double>(stats.ComputeAverageTimeNs() / kNsInUs);
  }

  [[nodiscard]] Baseline<double> BaselineExclusiveTimePerFrameUs(SFID sfid) const {
    return LiftAndApply(&TimePerFrameUs, BaselineExclusiveRate(sfid),
                        report_.GetBaselineFrameTrackStats());
  }
  [[nodiscard]] Comparison<double> ComparisonExclusiveTimePerFrameUs(SFID sfid) const {
    return LiftAndApply(&TimePerFrameUs, ComparisonExclusiveRate(sfid),
                        report_.GetComparisonFrameTrackStats());
  }

  [[nodiscard]] double GetPvalue(SFID sfid) const {
    const orbit_mizar_data::CorrectedComparisonResult& result = report_.GetComparisonResult(sfid);
    return is_multiplicity_correction_enabled_ ? result.corrected_pvalue : result.pvalue;
  }

  [[nodiscard]] static double Slowdown(Baseline<double> baseline_time,
                                       Comparison<double> comparison_time) {
    return *comparison_time - *baseline_time;
  }

  [[nodiscard]] double SlowdownPercent(SFID sfid) const {
    const Baseline<double> baseline_time = BaselineExclusiveTimePerFrameUs(sfid);
    const Comparison<double> comparison_time = ComparisonExclusiveTimePerFrameUs(sfid);
    return Slowdown(baseline_time, comparison_time) / *baseline_time * 100;
  }

  [[nodiscard]] double PercentOfFrameSlowdown(SFID sfid) const {
    const double function_slowdown_per_frame =
        Slowdown(BaselineExclusiveTimePerFrameUs(sfid), ComparisonExclusiveTimePerFrameUs(sfid));

    const Baseline<double> baseline_frame_time =
        orbit_base::LiftAndApply(&AverageFrameTime, report_.GetBaselineFrameTrackStats());
    const Comparison<double> comparison_frame_time =
        orbit_base::LiftAndApply(&AverageFrameTime, report_.GetComparisonFrameTrackStats());

    const double frame_slowdown = Slowdown(baseline_frame_time, comparison_frame_time);

    return function_slowdown_per_frame / std::abs(frame_slowdown) * 100;
  }

  [[nodiscard]] double MakeNumericEntry(const Index& index) const {
    const auto [sfid, column] = index;
    switch (column) {
      case Column::kBaselineExclusivePercent:
        return *BaselineExclusiveRate(sfid) * 100;
      case Column::kBaselineExclusiveTimePerFrame:
        return *BaselineExclusiveTimePerFrameUs(sfid);
      case Column::kComparisonExclusivePercent:
        return *ComparisonExclusiveRate(sfid) * 100;
      case Column::kComparisonExclusiveTimePerFrame:
        return *ComparisonExclusiveTimePerFrameUs(sfid);
      case Column::kPvalue:
      case Column::kIsSignificant:
        return GetPvalue(sfid);
      case Column::kSlowdownPercent:
        return SlowdownPercent(sfid);
      case Column::kPercentOfSlowdown:
        return PercentOfFrameSlowdown(sfid);
      default:
        ORBIT_UNREACHABLE();
    }
  }

  [[nodiscard]] const std::string& GetFunctionName(SFID sfid) const {
    const orbit_mizar_base::BaselineAndComparisonFunctionSymbols& symbols =
        report_.GetSfidToSymbols().at(sfid);
    switch (function_name_to_show_) {
      case FunctionNameToShow::kBaseline:
        return symbols.baseline_function_symbol->function_name;
      case FunctionNameToShow::kComparison:
        return symbols.comparison_function_symbol->function_name;
    }
    ORBIT_UNREACHABLE();
  }

  Report report_;
  std::vector<SFID> sfids_;
  bool is_multiplicity_correction_enabled_;
  double significance_level_;
  FunctionNameToShow function_name_to_show_;
};

using SamplingWithFrameTrackReportModel =
    SamplingWithFrameTrackReportModelTmpl<orbit_mizar_data::SamplingWithFrameTrackComparisonReport,
                                          orbit_mizar_data::SamplingCounts,
                                          orbit_client_data::ScopeStats>;

}  // namespace orbit_mizar_models

#endif  // MIZAR_MODELS_SAMPLING_WITH_FRAME_TRACK_REPORT_MODEL_H_