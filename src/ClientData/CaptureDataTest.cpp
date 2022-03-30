// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/numbers.h>
#include <absl/strings/str_split.h>
#include <absl/strings/string_view.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <optional>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ScopeIdConstants.h"
#include "ClientData/ScopeIdProvider.h"
#include "ClientData/ScopeStats.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/ReadFileToString.h"
#include "Test/Path.h"

namespace orbit_client_data {

namespace {

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

}  // namespace

constexpr size_t kTimersForFirstId = 3;
constexpr size_t kTimersForSecondId = 2;
constexpr size_t kTimerCount = kTimersForFirstId + kTimersForSecondId;
constexpr uint64_t kFirstId = 1;
constexpr uint64_t kSecondId = 2;
constexpr std::array<uint64_t, kTimerCount> kTimerIds = {kFirstId, kFirstId, kFirstId, kSecondId,
                                                         kSecondId};
constexpr std::array<uint64_t, kTimerCount> kStarts = {10, 20, 30, 40, 50};
constexpr std::array<uint64_t, kTimersForFirstId> kDurationsForFirstId = {300, 100, 200};
constexpr std::array<uint64_t, kTimersForSecondId> kDurationsForSecondId = {500, 400};

constexpr uint64_t kLargeInteger = 10'000'000'000'000'000;

static const std::array<uint64_t, kTimerCount> kDurations = [] {
  std::array<uint64_t, kTimerCount> result;
  std::copy(std::begin(kDurationsForFirstId), std::end(kDurationsForFirstId), std::begin(result));
  std::copy(std::begin(kDurationsForSecondId), std::end(kDurationsForSecondId),
            std::begin(result) + kTimersForFirstId);
  return result;
}();
static const std::array<TimerInfo, kTimerCount> kTimerInfos = [] {
  std::array<TimerInfo, kTimerCount> result;
  for (size_t i = 0; i < kTimerCount; ++i) {
    result[i].set_function_id(kTimerIds[i]);
    result[i].set_start(kStarts[i]);
    result[i].set_end(kStarts[i] + kDurations[i]);
  }
  return result;
}();

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

TEST_F(CaptureDataTest, VarianceIsCorrectForLongDurations) {
  for (TimerInfo timer : kTimerInfos) {
    timer.set_end(timer.end() + kLargeInteger);
    capture_data_.UpdateScopeStats(timer);
  }

  capture_data_.UpdateScopeStats(kTimerInfoWithInvalidScopeId);

  ORBIT_LOG("VARIANCE %f", capture_data_.GetScopeStatsOrDefault(kFirstId).variance_ns());
  EXPECT_NEAR(capture_data_.GetScopeStatsOrDefault(kFirstId).variance_ns(), kFirstVariance, 1.0);
  EXPECT_NEAR(capture_data_.GetScopeStatsOrDefault(kSecondId).variance_ns(), kSecondVariance, 1.0);
}

TEST_F(CaptureDataTest, VarianceIsCorrectOnScimitarDataset) {
  std::filesystem::path path = orbit_test::GetTestdataDir() / "scimitar_variance_and_durations.csv";
  const ErrorMessageOr<std::string> file_content_or_error = orbit_base::ReadFileToString(path);
  EXPECT_TRUE(file_content_or_error.has_value());
  const std::string& file_content = file_content_or_error.value();
  const std::vector<absl::string_view> tokens = absl::StrSplit(file_content, '\n');

  double expected_variance;
  EXPECT_TRUE(absl::SimpleAtod(*tokens.begin(), &expected_variance));

  std::vector<TimerInfo> timers;
  std::transform(std::begin(tokens) + 1, std::end(tokens) - 1, std::back_inserter(timers),
                 [](const absl::string_view& line) {
                   uint64_t duration = std::stoull(std::string(std::begin(line), std::end(line)));
                   TimerInfo timer;
                   timer.set_function_id(kFirstId);
                   timer.set_start(0);
                   timer.set_end(duration);
                   return timer;
                 });

  for (const TimerInfo& timer : timers) {
    capture_data_.UpdateScopeStats(timer);
  }

  const double actual_variance = capture_data_.GetScopeStatsOrDefault(kFirstId).variance_ns();

  EXPECT_LE(abs(actual_variance / expected_variance - 1.0), 1e-5);
}

}  // namespace orbit_client_data