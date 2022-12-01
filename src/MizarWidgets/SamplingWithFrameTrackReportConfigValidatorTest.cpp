
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <variant>

#include "ClientData/ScopeId.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarWidgets/SamplingWithFrameTrackReportConfigValidator.h"
#include "OrbitBase/Typedef.h"
#include "TestUtils/TestUtils.h"

using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::RelativeTimeNs;
using ::orbit_mizar_base::TID;
using ::orbit_mizar_base::TimestampNs;
using ::orbit_mizar_data::FrameTrackId;
using ::orbit_mizar_data::FrameTrackInfo;
using ::orbit_test_utils::HasError;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

class MockPairedData {
 public:
  MOCK_METHOD(RelativeTimeNs, CaptureDurationNs, (), (const));
};

class MockBaselineAndComparison {
 public:
  MOCK_METHOD(const Baseline<MockPairedData>&, GetBaselineData, (), (const));
  MOCK_METHOD(const Comparison<MockPairedData>&, GetComparisonData, (), (const));
};

}  // namespace

namespace orbit_mizar_widgets {

constexpr RelativeTimeNs kBaselineCaptureDuration(123456);
constexpr RelativeTimeNs kComparisonCaptureDuration(234567);

using HalfConfig = orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig;

TEST(SamplingWithFrameTrackReportConfigValidator, IsCorrect) {
  Baseline<MockPairedData> baseline_data;
  Comparison<MockPairedData> comparison_data;
  MockBaselineAndComparison bac;

  EXPECT_CALL(*baseline_data, CaptureDurationNs).WillRepeatedly(Return(kBaselineCaptureDuration));
  EXPECT_CALL(*comparison_data, CaptureDurationNs)
      .WillRepeatedly(Return(kComparisonCaptureDuration));

  EXPECT_CALL(bac, GetBaselineData).WillRepeatedly(ReturnRef(baseline_data));
  EXPECT_CALL(bac, GetComparisonData).WillRepeatedly(ReturnRef(comparison_data));

  const SamplingWithFrameTrackReportConfigValidatorTmpl<MockBaselineAndComparison, MockPairedData>
      validator{};

  constexpr FrameTrackId kId(orbit_client_data::ScopeId(0));
  constexpr RelativeTimeNs kStart(0);

  EXPECT_THAT(
      validator.Validate(
          &bac,
          orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{TID(1)}, kStart, kId),
          orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{}, kStart, kId)),
      HasError("Comparison: No threads selected"));

  EXPECT_THAT(
      validator.Validate(
          &bac, orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{}, kStart, kId),
          orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{TID(1)}, kStart,
                                                       kId)),
      HasError("Baseline: No threads selected"));

  constexpr RelativeTimeNs kOneNs(1);

  EXPECT_THAT(validator.Validate(
                  &bac,
                  orbit_mizar_base::MakeBaseline<HalfConfig>(
                      absl::flat_hash_set<TID>{TID(1)}, Add(kBaselineCaptureDuration, kOneNs), kId),
                  orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{TID(1)},
                                                               kStart, kId)),
              HasError("Baseline: Start > capture duration"));

  EXPECT_THAT(
      validator.Validate(
          &bac,
          orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{TID(1)}, kStart, kId),
          orbit_mizar_base::MakeComparison<HalfConfig>(
              absl::flat_hash_set<TID>{TID(1)}, Add(kComparisonCaptureDuration, kOneNs), kId)),
      HasError("Comparison: Start > capture duration"));
}

}  // namespace orbit_mizar_widgets