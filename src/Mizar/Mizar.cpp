// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include <iostream>
#include <memory>
#include <optional>
#include <string>

#include "CaptureClient/LoadCapture.h"
#include "CaptureFile/CaptureFile.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/MizarData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Logging.h"

template <typename T>
using Baseline = ::orbit_mizar_base::Baseline<T>;

template <typename T>
using Comparison = ::orbit_mizar_base::Comparison<T>;

using ::orbit_mizar_base::MakeBaseline;
using ::orbit_mizar_base::MakeComparison;

[[nodiscard]] static ErrorMessageOr<void> LoadCapture(orbit_mizar_data::MizarData* data,
                                                      std::filesystem::path path) {
  OUTCOME_TRY(auto capture_file, orbit_capture_file::CaptureFile::OpenForReadWrite(path));
  std::atomic<bool> capture_loading_cancellation_requested = false;

  // The treatment is the same for CaptureOutcome::kComplete, CaptureOutcome::kCancelled
  std::ignore = orbit_capture_client::LoadCapture(data, capture_file.get(),
                                                  &capture_loading_cancellation_requested);
  return outcome::success();
}

ABSL_FLAG(std::string, baseline_path, "", "The path to the baseline capture file");
ABSL_FLAG(std::string, comparison_path, "", "The path to the comparison capture file");
ABSL_FLAG(uint32_t, baseline_tid, 0, "Frame track TID for baseline");
ABSL_FLAG(uint32_t, comparison_tid, 0, "The path to the comparison capture file");
ABSL_FLAG(uint64_t, baseline_start_ms, 0, "Start time in ms for baseline");
ABSL_FLAG(uint64_t, comparison_start_ms, 0, "Start time in ms for comparison");

static std::string ExpandPathHomeFolder(const std::string& path) {
  const std::string kHomeForderEnvVariable = "HOME";
  if (path[0] == '~') return getenv(kHomeForderEnvVariable.c_str()) + path.substr(1);
  return path;
}

int main(int argc, char** argv) {
  // The main in its current state is used to testing/experimenting and serves no other purpose
  absl::ParseCommandLine(argc, argv);

  const std::filesystem::path baseline_path =
      ExpandPathHomeFolder(absl::GetFlag(FLAGS_baseline_path));
  const std::filesystem::path comparison_path =
      ExpandPathHomeFolder(absl::GetFlag(FLAGS_comparison_path).substr(1));
  const uint32_t baseline_tid = absl::GetFlag(FLAGS_baseline_tid);
  const uint32_t comparison_tid = absl::GetFlag(FLAGS_comparison_tid);

  constexpr uint64_t kNsInMs = 1'000'000;
  const uint64_t baseline_start_ns = absl::GetFlag(FLAGS_baseline_start_ms) * kNsInMs;
  const uint64_t comparison_start_ns = absl::GetFlag(FLAGS_comparison_start_ms) * kNsInMs;

  auto baseline = std::make_unique<orbit_mizar_data::MizarData>();
  auto comparison = std::make_unique<orbit_mizar_data::MizarData>();

  auto baseline_error_message = LoadCapture(baseline.get(), baseline_path);
  if (baseline_error_message.has_error()) {
    ORBIT_ERROR("%s", baseline_error_message.error().message());
    return 1;
  }

  auto comparison_error_message = LoadCapture(comparison.get(), comparison_path);
  if (comparison_error_message.has_error()) {
    ORBIT_ERROR("%s", comparison_error_message.error().message());
    return 1;
  }

  orbit_mizar_data::BaselineAndComparison bac =
      CreateBaselineAndComparison(std::move(baseline), std::move(comparison));

  constexpr uint64_t kDuration = std::numeric_limits<uint64_t>::max();
  constexpr uint64_t kFrameTrackScopeId = 1;

  const orbit_mizar_data::SamplingWithFrameTrackComparisonReport report =
      bac.MakeSamplingWithFrameTrackReport(
          MakeBaseline<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
              absl::flat_hash_set<uint32_t>{baseline_tid}, baseline_start_ns, kDuration,
              kFrameTrackScopeId),
          MakeComparison<orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
              absl::flat_hash_set<uint32_t>{comparison_tid}, comparison_start_ns, kDuration,
              kFrameTrackScopeId));

  for (const auto& [sfid, name] : bac.sfid_to_name()) {
    const uint64_t baseline_cnt = report.GetBaselineSamplingCounts()->GetExclusiveCount(sfid);
    const uint64_t comparison_cnt = report.GetComparisonSamplingCounts()->GetExclusiveCount(sfid);
    const double pvalue = report.GetComparisonResult(sfid).pvalue;
    if (pvalue < 0.05) {
      ORBIT_LOG("%s %.2f %s %u %u", name, pvalue, static_cast<std::string>(sfid), baseline_cnt,
                comparison_cnt);
    }
  }
  ORBIT_LOG("Callstack count %u vs %u ", report.GetBaselineSamplingCounts()->GetTotalCallstacks(),
            report.GetComparisonSamplingCounts()->GetTotalCallstacks());
  ORBIT_LOG("Total number of common names %u  ", bac.sfid_to_name().size());
  ORBIT_LOG("Baseline mean frametime %u ns, stddev %u",
            report.GetBaselineFrameTrackStats()->ComputeAverageTimeNs(),
            report.GetBaselineFrameTrackStats()->ComputeStdDevNs());
  ORBIT_LOG("Comparison mean frametime %u ns, stddev %u",
            report.GetComparisonFrameTrackStats()->ComputeAverageTimeNs(),
            report.GetComparisonFrameTrackStats()->ComputeStdDevNs());
  return 0;
}