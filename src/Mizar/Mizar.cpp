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
  if (argc < 3) {
    ORBIT_ERROR("Two filepaths should be given");
    return 1;
  }
  auto baseline = std::make_unique<orbit_mizar_data::MizarData>();
  auto comparison = std::make_unique<orbit_mizar_data::MizarData>();

  const std::filesystem::path baseline_path = argv[1];
  const std::filesystem::path comparison_path = argv[2];

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

  constexpr uint64_t kStart = 20'000'000'000;
  constexpr uint64_t kDuration = std::numeric_limits<uint64_t>::max();

  const orbit_mizar_data::SamplingWithFrameTrackComparisonReport report =
      bac.MakeSamplingWithFrameTrackReport(
          orbit_mizar_data::BaselineSamplingWithFrameTrackReportConfig{
              {orbit_base::kAllProcessThreadsTid}, kStart, kDuration, 1},
          orbit_mizar_data::ComparisonSamplingWithFrameTrackReportConfig{
              {orbit_base::kAllProcessThreadsTid}, kStart, kDuration, 1});

  for (const auto& [sfid, name] : bac.sfid_to_name()) {
    const uint64_t baseline_cnt = report.baseline_sampling_counts.GetExclusiveCount(sfid);
    const uint64_t comparison_cnt = report.baseline_sampling_counts.GetExclusiveCount(sfid);
    if (baseline_cnt > 0 || comparison_cnt > 0) {
      ORBIT_LOG("%s %s %.2f %.2f", name, static_cast<std::string>(sfid), baseline_cnt,
                comparison_cnt);
    }
  }
  ORBIT_LOG("Total number of common names %u  ", bac.sfid_to_name().size());
  ORBIT_LOG("Baseline mean frametime %u ns, stddev %u",
            report.baseline_frame_track_stats.ComputeAverageTimeNs(),
            report.baseline_frame_track_stats.ComputeStdDevNs());
  ORBIT_LOG("Comparison mean frametime %u ns, stddev %u",
            report.comparison_frame_track_stats.ComputeAverageTimeNs(),
            report.comparison_frame_track_stats.ComputeStdDevNs());
  return 0;
}