// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>

#include <iostream>
#include <memory>
#include <string>

#include "CaptureClient/LoadCapture.h"
#include "CaptureFile/CaptureFile.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/BaselineOrComparison.h"
#include "MizarData/MizarData.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "OrbitBase/Logging.h"

[[nodiscard]] static ErrorMessageOr<void> LoadCapture(orbit_mizar_data::MizarData* data,
                                                      std::filesystem::path path) {
  OUTCOME_TRY(auto capture_file, orbit_capture_file::CaptureFile::OpenForReadWrite(path));
  std::atomic<bool> capture_loading_cancellation_requested = false;

  // The treatment is the same for CaptureOutcome::kComplete, CaptureOutcome::kCancelled
  std::ignore = orbit_capture_client::LoadCapture(data, capture_file.get(),
                                                  &capture_loading_cancellation_requested);
  return outcome::success();
}

int main(int argc, char* argv[]) {
  // The main in its current state is used to testing/experimenting and serves no other purpose
  if (argc < 7) {
    ORBIT_ERROR(
        "Example: \n "
        "$./Mizar path/to/baseline baseline_tid baseline_start_ms path/to/comparison "
        "comparison_tid comparison_start_ms");
    return 1;
  }
  auto baseline = std::make_unique<orbit_mizar_data::MizarData>();
  auto comparison = std::make_unique<orbit_mizar_data::MizarData>();

  const std::filesystem::path baseline_path = argv[1];
  const std::filesystem::path comparison_path = argv[4];
  const uint32_t baseline_tid = static_cast<uint32_t>(std::stoul(argv[2]));
  const uint32_t comparison_tid = static_cast<uint32_t>(std::stoul(argv[5]));

  constexpr uint64_t kNsInMs = 1'000'000;
  const uint64_t baseline_start_ns = static_cast<uint64_t>(std::stoull(argv[3])) * kNsInMs;
  const uint64_t comparison_start_ns = static_cast<uint64_t>(std::stoull(argv[6])) * kNsInMs;

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

  const orbit_mizar_data::SamplingWithFrameTrackComparisonReport report =
      bac.MakeSamplingWithFrameTrackReport(
          orbit_mizar_data::MakeBaseline<
              orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
              absl::flat_hash_set<uint32_t>{baseline_tid}, baseline_start_ns, kDuration, 1),
          orbit_mizar_data::MakeComparison<
              orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig>(
              absl::flat_hash_set<uint32_t>{comparison_tid}, comparison_start_ns, kDuration, 1));

  for (const auto& [sfid, name] : bac.sfid_to_name()) {
    const uint64_t baseline_cnt =
        report.GetSamplingCounts<orbit_mizar_data::Baseline>()->GetExclusiveCount(sfid);
    const uint64_t comparison_cnt =
        report.GetSamplingCounts<orbit_mizar_data::Comparison>()->GetExclusiveCount(sfid);
    const double pvalue = report.GetComparisonResult(sfid).pvalue;
    if (pvalue < 0.05) {
      ORBIT_LOG("%s %.2f %s %u %u", name, pvalue, static_cast<std::string>(sfid), baseline_cnt,
                comparison_cnt);
    }
  }
  ORBIT_LOG("Callstack count %u vs %u ",
            report.GetSamplingCounts<orbit_mizar_data::Baseline>()->GetTotalCallstacks(),
            report.GetSamplingCounts<orbit_mizar_data::Comparison>()->GetTotalCallstacks());
  ORBIT_LOG("Total number of common names %u  ", bac.sfid_to_name().size());
  ORBIT_LOG("Baseline mean frametime %u ns, stddev %u",
            report.GetFrameTrackStats<orbit_mizar_data::Baseline>()->ComputeAverageTimeNs(),
            report.GetFrameTrackStats<orbit_mizar_data::Baseline>()->ComputeStdDevNs());
  ORBIT_LOG("Comparison mean frametime %u ns, stddev %u",
            report.GetFrameTrackStats<orbit_mizar_data::Comparison>()->ComputeAverageTimeNs(),
            report.GetFrameTrackStats<orbit_mizar_data::Comparison>()->ComputeStdDevNs());
  return 0;
}