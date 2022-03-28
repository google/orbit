
// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <optional>

#include "ClientData/CaptureData.h"
#include "ClientData/ScopeIdConstants.h"
#include "ClientData/ScopeIdProvider.h"
#include "ClientData/ScopeStats.h"
#include "GrpcProtos/capture.pb.h"
#include "SharedTestConstants.h"

namespace orbit_client_data {

class MockScopeIdProvider : public ScopeIdProvider {
 public:
  MOCK_METHOD(uint64_t, ProvideId, (const TimerInfo& timer_info));
  MOCK_METHOD(const std::string&, GetScopeName, (uint64_t scope_id), (const));
};

class CaptureDataTest : public testing::Test {
 public:
  explicit CaptureDataTest()
      : capture_data_{capture_started_, std::nullopt, {}, CaptureData::DataSource::kLiveCapture} {
    EXPECT_CALL(scope_id_provider_, ProvideId)
        .WillRepeatedly(testing::Invoke([](const TimerInfo& timer_info) {
          if (timer_info.function_id() == 0) return kInvalidScopeId;
          return timer_info.function_id();
        }));
  }

 protected:
  MockScopeIdProvider scope_id_provider_;
  orbit_grpc_protos::CaptureStarted capture_started_;
  CaptureData capture_data_;
};

constexpr double kFirstVariance = 6666.66666;
constexpr double kSecondVariance = 2500.0;

template <std::size_t Size>
[[nodiscard]] static ScopeStats GetStats(const std::array<uint64_t, Size>& durations,
                                         double variance) {
  ScopeStats stats{};
  stats.set_count(Size);
  const auto begin = std::begin(durations);
  const auto end = std::end(durations);

  stats.set_total_time_ns(std::reduce(begin, end));
  stats.set_min_ns(*std::min_element(begin, end));
  stats.set_max_ns(*std::max_element(begin, end));
  stats.set_variance_ns(variance);
  stats.set_std_dev_ns(static_cast<uint64_t>(std::sqrt(variance)));

  return stats;
}

const TimerInfo kTimerInfoWithInvalidScopeId = []() {
  TimerInfo timer;
  timer.set_start(0);
  timer.set_end(std::numeric_limits<uint64_t>::max());
  timer.set_function_id(0);
  return timer;
}();

static void ExpectStatsEqual(const ScopeStats& actual, const ScopeStats& other) {
  EXPECT_EQ(actual.total_time_ns(), other.total_time_ns());
  EXPECT_EQ(actual.min_ns(), other.min_ns());
  EXPECT_EQ(actual.max_ns(), other.max_ns());

  EXPECT_NEAR(actual.variance_ns(), other.variance_ns(), 1.0);
  EXPECT_NEAR(actual.std_dev_ns(), other.std_dev_ns(), 1.0);
}

TEST_F(CaptureDataTest, UpdateScopeStatsIsCorrect) {
  for (const TimerInfo& timer : kTimerInfos) {
    capture_data_.UpdateScopeStats(timer);
  }

  capture_data_.UpdateScopeStats(kTimerInfoWithInvalidScopeId);

  ExpectStatsEqual(capture_data_.GetScopeStatsOrDefault(kFirstId),
                   GetStats(kDurationsForFirstId, kFirstVariance));
  ExpectStatsEqual(capture_data_.GetScopeStatsOrDefault(kSecondId),
                   GetStats(kDurationsForSecondId, kSecondVariance));
}

}  // namespace orbit_client_data