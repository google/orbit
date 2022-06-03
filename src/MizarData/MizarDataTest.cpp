// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <google/protobuf/util/message_differencer.h>
#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <iterator>
#include <optional>
#include <string>
#include <vector>

#include "ClientData/CallstackInfo.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/ScopeInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarData/MizarData.h"

using ::testing::Invoke;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedPointwise;

namespace orbit_mizar_data {

using google::protobuf::util::MessageDifferencer;

constexpr size_t kTimersNum = 5;
constexpr std::array<uint64_t, kTimersNum> kStarts{10, 20, 30, 40, 50};
constexpr std::array<uint64_t, kTimersNum> kEnds{110, 220, 330, 440, 550};
// These are the timer_info types that are stored in MizarData
constexpr std::array<TimerInfo::Type, kTimersNum> kTypesToStore{
    TimerInfo::kNone, TimerInfo::kNone, TimerInfo::kNone, TimerInfo::kApiScope,
    TimerInfo::kApiScope};
constexpr std::array<TimerInfo::Type, kTimersNum> kTypesToIgnore{
    TimerInfo::kGpuActivity, TimerInfo::kApiEvent, TimerInfo::kApiEvent, TimerInfo::kApiScopeAsync,
    TimerInfo::kApiEvent};
constexpr uint64_t kFunctionId = 1;
const std::string kFunctionName = "foo()";
const std::string kMSName = "ManualScope";
constexpr uint32_t kTID = 123;

static std::array<TimerInfo, kTimersNum> MakeTimerInfos(
    const std::array<TimerInfo::Type, kTimersNum>& types) {
  std::array<TimerInfo, kTimersNum> result;
  for (size_t i = 0; i < kTimersNum; ++i) {
    result[i].set_thread_id(kTID);
    result[i].set_start(kStarts[i]);
    result[i].set_end(kEnds[i]);
    TimerInfo::Type type = types[i];
    result[i].set_type(type);
    if (type == TimerInfo::kNone) {
      result[i].set_function_id(kFunctionId);
    } else {
      result[i].set_api_scope_name(kMSName);
    }
  }
  return result;
}

const std::array<TimerInfo, kTimersNum> kTimersToStore = MakeTimerInfos(kTypesToStore);
const std::array<TimerInfo, kTimersNum> kTimersToIgnore = MakeTimerInfos(kTypesToIgnore);

const orbit_grpc_protos::CaptureStarted kCaptureStarted = [] {
  orbit_grpc_protos::CaptureStarted result;
  orbit_grpc_protos::InstrumentedFunction* function =
      result.mutable_capture_options()->add_instrumented_functions();
  function->set_function_id(kFunctionId);
  function->set_function_name(kFunctionName);
  return result;
}();

const absl::flat_hash_set<orbit_client_data::ScopeType> kStoredScopeTypes = {
    orbit_client_data::ScopeType::kApiScope,
    orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction};

static void CallOnCaptureStarted(MizarData& data) {
  orbit_grpc_protos::CaptureStarted capture_started;
  capture_started.capture_options().instrumented_functions();
  data.OnCaptureStarted(kCaptureStarted, "path/to/file", {});
}

MATCHER(TimerInfosEq, "") {
  const TimerInfo& a = std::get<0>(arg);
  const TimerInfo& b = std::get<1>(arg);

  return MessageDifferencer::Equivalent(a, b);
}

TEST(MizarDataTest, OnCaptureStartedInitializesCaptureData) {
  MizarData data;
  EXPECT_FALSE(data.HasCaptureData());

  CallOnCaptureStarted(data);

  EXPECT_TRUE(data.HasCaptureData());
}

TEST(MizarDataTest, OnTimerAddsDAndMSAndOnlyThem) {
  MizarData data;
  CallOnCaptureStarted(data);
  for (const TimerInfo& timer : kTimersToStore) {
    data.OnTimer(timer);
  }
  for (const TimerInfo& timer : kTimersToIgnore) {
    data.OnTimer(timer);
  }
  data.OnCaptureFinished({});

  std::vector<const TimerInfo*> stored_timers_ptrs =
      data.GetCaptureData().GetAllScopeTimers(kStoredScopeTypes);
  std::vector<TimerInfo> stored_timers;
  std::transform(std::begin(stored_timers_ptrs), std::end(stored_timers_ptrs),
                 std::back_inserter(stored_timers), [](const TimerInfo* ptr) { return (*ptr); });

  EXPECT_THAT(stored_timers, UnorderedPointwise(TimerInfosEq(), kTimersToStore));
}

constexpr uint64_t kFunctionAddress = 0xBEAF;
constexpr uint64_t kAnotherFunctionAddress = 0xF00D;
constexpr uint64_t kUnknownFunctionAddress = 0xBAD;
const orbit_client_data::LinuxAddressInfo kLinuxAddressInfo(kFunctionAddress, 0, "/module/path",
                                                            kFunctionName);

TEST(MizarDataTest, GetFunctionNameFromAddressIsCorrect) {
  MizarData data;
  CallOnCaptureStarted(data);

  data.OnAddressInfo(kLinuxAddressInfo);
  std::optional<std::string> name_optional = data.GetFunctionNameFromAddress(kFunctionAddress);
  EXPECT_TRUE(name_optional.has_value());
  EXPECT_EQ(name_optional.value(), kFunctionName);

  EXPECT_FALSE(data.GetFunctionNameFromAddress(kUnknownFunctionAddress));
}

class MockMizarData : public MizarData {
 public:
  MOCK_METHOD(std::optional<std::string>, GetFunctionNameFromAddress, (uint64_t), (const override));
};

const uint64_t kTime = 951753;
const orbit_client_data::CallstackInfo kCallstackInfo({kFunctionAddress, kUnknownFunctionAddress,
                                                       kFunctionAddress},
                                                      orbit_client_data::CallstackType::kComplete);
constexpr uint64_t kCallstackId = 0xCA11;
static const orbit_client_data::CallstackEvent kCallstackEvent(kTime, kCallstackId, kTID);
static const absl::flat_hash_map<uint64_t, std::string> kSymbolsTable = {
    {kFunctionAddress, kFunctionName}, {kAnotherFunctionAddress, "food()"}};

TEST(MizarDataTest, AllAddressToNameIsCorrect) {
  MockMizarData data;

  EXPECT_CALL(data, GetFunctionNameFromAddress)
      .WillRepeatedly(Invoke([](uint64_t addr) -> std::optional<std::string> {
        if (const auto it = kSymbolsTable.find(addr); it != kSymbolsTable.end()) {
          return it->second;
        }
        return std::nullopt;
      }));

  CallOnCaptureStarted(data);
  data.OnUniqueCallstack(kCallstackId, kCallstackInfo);
  data.OnCallstackEvent(kCallstackEvent);
  data.OnCaptureFinished({});

  EXPECT_THAT(data.AllAddressToName(),
              UnorderedElementsAre(std::make_pair(kFunctionAddress, kFunctionName)));
}
}  // namespace orbit_mizar_data
