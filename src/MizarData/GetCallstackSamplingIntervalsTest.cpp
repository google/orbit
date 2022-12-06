// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <memory>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "MizarBase/ThreadId.h"
#include "MizarData/GetCallstackSamplingIntervals.h"
#include "OrbitBase/Append.h"
#include "OrbitBase/Typedef.h"

namespace orbit_mizar_data {

using ::orbit_client_data::CallstackData;
using ::orbit_mizar_base::TID;
using ::testing::IsEmpty;
using ::testing::IsSupersetOf;
using ::testing::UnorderedElementsAreArray;

constexpr TID kFirstTID(1);
constexpr TID kSecondTID(2);

static const std::vector<uint64_t> kFirstThreadTimestamps = {10, 22, 34, 48, 62};
static const std::vector<uint64_t> kSecondThreadTimestamps = {30, 80, 150, 210};

constexpr uint64_t kMinTimestamp = 0;
constexpr uint64_t kMaxTimestamp = std::numeric_limits<uint64_t>::max();

constexpr uint64_t kCallstackSampleId = 0;

const orbit_client_data::CallstackInfo kCallstackInfo({},
                                                      orbit_client_data::CallstackType::kComplete);

[[nodiscard]] static std::vector<uint64_t> GetExpectedIntervals(
    absl::Span<const uint64_t> timestamps) {
  std::vector<uint64_t> intervals;
  for (size_t i = 1; i < timestamps.size(); ++i) {
    intervals.push_back(timestamps[i] - timestamps[i - 1]);
  }
  return intervals;
}

class GetCallstackSamplingIntervalsTest : public ::testing::Test {
 public:
  GetCallstackSamplingIntervalsTest() {
    callstack_data_->AddUniqueCallstack(kCallstackSampleId, kCallstackInfo);
  }

  void PopulateCallstackData(absl::Span<const uint64_t> timestamps, TID tid) const {
    for (const uint64_t timestamp : timestamps) {
      callstack_data_->AddCallstackEvent({timestamp, kCallstackSampleId, *tid});
    }
  }

  [[nodiscard]] std::vector<uint64_t> GetActualIntervals(
      const absl::flat_hash_set<TID>& tids) const {
    return GetSamplingIntervalsNs(tids, kMinTimestamp, kMaxTimestamp, *callstack_data_);
  }

  void ExpectEmptyVectorReturnedForAllTids() const {
    EXPECT_THAT(GetActualIntervals({kFirstTID, kSecondTID}), IsEmpty());
    EXPECT_THAT(GetActualIntervals({kFirstTID}), IsEmpty());
    EXPECT_THAT(GetActualIntervals({kSecondTID}), IsEmpty());
    EXPECT_THAT(GetActualIntervals({}), IsEmpty());
  }

  std::unique_ptr<CallstackData> callstack_data_ = std::make_unique<CallstackData>();
};

TEST_F(GetCallstackSamplingIntervalsTest, ReturnsEmptyForNoEvents) {
  ExpectEmptyVectorReturnedForAllTids();
}

TEST_F(GetCallstackSamplingIntervalsTest, ReturnsEmptyForNoEventsSingleEventPerThread) {
  PopulateCallstackData({0}, kFirstTID);
  PopulateCallstackData({0}, kSecondTID);

  ExpectEmptyVectorReturnedForAllTids();
}

TEST_F(GetCallstackSamplingIntervalsTest, SingleEventInOneThreadAndMultipleInTheOther) {
  PopulateCallstackData({0}, kFirstTID);
  PopulateCallstackData(kSecondThreadTimestamps, kSecondTID);

  EXPECT_THAT(GetActualIntervals({kFirstTID, kSecondTID}),
              UnorderedElementsAreArray(GetExpectedIntervals(kSecondThreadTimestamps)));
  EXPECT_THAT(GetActualIntervals({kFirstTID}), IsEmpty());
  EXPECT_THAT(GetActualIntervals({kSecondTID}),
              UnorderedElementsAreArray(GetExpectedIntervals(kSecondThreadTimestamps)));
  EXPECT_THAT(GetActualIntervals({}), IsEmpty());
}

TEST_F(GetCallstackSamplingIntervalsTest, MultipleEventsPerThread) {
  PopulateCallstackData(kFirstThreadTimestamps, kFirstTID);
  PopulateCallstackData(kSecondThreadTimestamps, kSecondTID);

  std::vector<uint64_t> expected_intervals_for_both_callstacks =
      GetExpectedIntervals(kFirstThreadTimestamps);
  orbit_base::Append(expected_intervals_for_both_callstacks,
                     GetExpectedIntervals(kSecondThreadTimestamps));

  EXPECT_THAT(GetActualIntervals({kFirstTID, kSecondTID}),
              UnorderedElementsAreArray(expected_intervals_for_both_callstacks));
  EXPECT_THAT(GetActualIntervals({kFirstTID}),
              UnorderedElementsAreArray(GetExpectedIntervals(kFirstThreadTimestamps)));
  EXPECT_THAT(GetActualIntervals({kSecondTID}),
              UnorderedElementsAreArray(GetExpectedIntervals(kSecondThreadTimestamps)));
  EXPECT_THAT(GetActualIntervals({}), IsEmpty());
}

}  // namespace orbit_mizar_data