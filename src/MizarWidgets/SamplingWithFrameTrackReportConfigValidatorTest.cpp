
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "MizarBase/BaselineOrComparison.h"
#include "MizarWidgets/SamplingWithFrameTrackReportConfigValidator.h"
#include "QString"

using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::TID;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

class MockPairedData {
 public:
  MOCK_METHOD(uint64_t, CaptureDuration, (), (const));
};

class MockBaselineAndComparison {
 public:
  MOCK_METHOD(const Baseline<MockPairedData>&, GetBaselineData, (), (const));
  MOCK_METHOD(const Comparison<MockPairedData>&, GetComparisonData, (), (const));
};

QString last_reported_error_message;
void MockErrorReporter(const QString& message) { last_reported_error_message = message; }

}  // namespace

namespace orbit_mizar_widgets {

constexpr uint64_t kBaselineCaptureDuration = 123456;
constexpr uint64_t kComparisonCaptureDuration = 234567;
const QString kBaselineTitleStr = "Baseline";
const QString kComparisonTitleStr = "Comparison";
const Baseline<QString> kBaselineTitle{kBaselineTitleStr};
const Comparison<QString> kComparisonTitle{kComparisonTitleStr};

using HalfConfig = orbit_mizar_data::HalfOfSamplingWithFrameTrackReportConfig;

TEST(SamplingWithFrameTrackReportConfigValidator, IsCorrect) {
  Baseline<MockPairedData> baseline_data;
  Comparison<MockPairedData> comparison_data;
  MockBaselineAndComparison bac;

  EXPECT_CALL(*baseline_data, CaptureDuration).WillRepeatedly(Return(kBaselineCaptureDuration));
  EXPECT_CALL(*comparison_data, CaptureDuration).WillRepeatedly(Return(kComparisonCaptureDuration));

  EXPECT_CALL(bac, GetBaselineData).WillRepeatedly(ReturnRef(baseline_data));
  EXPECT_CALL(bac, GetComparisonData).WillRepeatedly(ReturnRef(comparison_data));

  SamplingWithFrameTrackReportConfigValidatorTmpl<MockBaselineAndComparison, MockPairedData,
                                                  MockErrorReporter>
      validator{kBaselineTitle, kComparisonTitle};

  EXPECT_FALSE(validator.Validate(
      &bac,
      orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{TID(1)}, /*start_ns=*/0,
                                                 /*duration_ns=*/0, /*frame_track_scope_id=*/0),
      orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{},
                                                   /*start_ns=*/0, /*duration_ns=*/0,
                                                   /*frame_track_scope_id=*/0)));
  EXPECT_EQ(last_reported_error_message.toStdString(), "Comparison: No threads selected");

  EXPECT_FALSE(validator.Validate(
      &bac,
      orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{}, /*start_ns=*/0,
                                                 /*duration_ns=*/0, /*frame_track_scope_id=*/0),
      orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{TID(1)},
                                                   /*start_ns=*/0, /*duration_ns=*/0,
                                                   /*frame_track_scope_id=*/0)));
  EXPECT_EQ(last_reported_error_message.toStdString(), "Baseline: No threads selected");

  EXPECT_FALSE(validator.Validate(
      &bac,
      orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{TID(1)},
                                                 /*start_ns=*/kBaselineCaptureDuration + 1,
                                                 /*duration_ns=*/0,
                                                 /*frame_track_scope_id=*/0),
      orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{TID(1)},
                                                   /*start_ns=*/0, /*duration_ns=*/0,
                                                   /*frame_track_scope_id=*/0)));
  EXPECT_EQ(last_reported_error_message.toStdString(), "Baseline: Start > capture duration");

  EXPECT_FALSE(validator.Validate(
      &bac,
      orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{TID(1)}, /*start_ns=*/0,
                                                 /*duration_ns=*/0,
                                                 /*frame_track_scope_id=*/0),
      orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{TID(1)},
                                                   /*start_ns=*/kComparisonCaptureDuration + 1,
                                                   /*duration_ns=*/0,
                                                   /*frame_track_scope_id=*/0)));
  EXPECT_EQ(last_reported_error_message.toStdString(), "Comparison: Start > capture duration");

  EXPECT_TRUE(validator.Validate(
      &bac,
      orbit_mizar_base::MakeBaseline<HalfConfig>(absl::flat_hash_set<TID>{TID(1)}, /*start_ns=*/0,
                                                 /*duration_ns=*/kBaselineCaptureDuration - 1,
                                                 /*frame_track_scope_id=*/0),
      orbit_mizar_base::MakeComparison<HalfConfig>(absl::flat_hash_set<TID>{TID(1)},
                                                   /*start_ns=*/kComparisonCaptureDuration - 1,
                                                   /*duration_ns=*/0,
                                                   /*frame_track_scope_id=*/0)));
}

}  // namespace orbit_mizar_widgets