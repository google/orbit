// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "MizarBase/AbsoluteAddress.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/MizarPairedData.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_grpc_protos::PresentEvent;
using ::orbit_mizar_base::AbsoluteAddress;
using ::orbit_mizar_base::ForEachFrame;
using ::orbit_mizar_base::RelativeTimeNs;
using ::orbit_mizar_base::SFID;
using ::orbit_mizar_base::TID;
using ::orbit_mizar_base::TimestampNs;
using ::orbit_test_utils::MakeMap;
using ::std::chrono::milliseconds;
using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

namespace {

class MockCaptureData {
 public:
  MOCK_METHOD(const orbit_client_data::CallstackData&, GetCallstackData, (), (const));
  MOCK_METHOD(std::vector<const TimerInfo*>, GetTimersForScope, (ScopeId, uint64_t, uint64_t),
              (const));
  MOCK_METHOD((const absl::flat_hash_map<uint32_t, std::string>&), thread_names, (), (const));
  MOCK_METHOD(std::vector<ScopeId>, GetAllProvidedScopeIds, (), (const));
  MOCK_METHOD(orbit_client_data::ScopeInfo, GetScopeInfo, (ScopeId scope_id), (const));
};

class MockMizarData {
 public:
  MOCK_METHOD(const MockCaptureData&, GetCaptureData, (), (const));
  MOCK_METHOD(TimestampNs, GetCaptureStartTimestampNs, (), (const));
  MOCK_METHOD(RelativeTimeNs, GetNominalSamplingPeriodNs, (), (const));
  MOCK_METHOD((absl::flat_hash_map<PresentEvent::Source, std::vector<PresentEvent>>),
              source_to_present_events, (), (const));
};

struct MockFrameTrackStats {
  MockFrameTrackStats() = default;

  void UpdateStats(uint64_t duration) {
    durations_fed_since_last_instantiation_.emplace_back(duration);
  }

  std::vector<RelativeTimeNs> durations_fed_since_last_instantiation_;
};

}  // namespace

namespace orbit_mizar_data {
constexpr AbsoluteAddress kAddressFood(0xF00D);
constexpr AbsoluteAddress kAddressBad(0xBAD);
constexpr AbsoluteAddress kAddressCall(0xCA11);
constexpr AbsoluteAddress kAddressBefore(0xB3F0);

const orbit_client_data::CallstackInfo kCompleteCallstack(
    {*kAddressBefore, *kAddressCall, *kAddressBad}, orbit_client_data::CallstackType::kComplete);
const orbit_client_data::CallstackInfo kInCompleteCallstack(
    {*kAddressBefore, *kAddressCall, *kAddressBad},
    orbit_client_data::CallstackType::kDwarfUnwindingError);
const orbit_client_data::CallstackInfo kAnotherCompleteCallstack(
    {*kAddressBefore, *kAddressCall, *kAddressFood}, orbit_client_data::CallstackType::kComplete);

constexpr uint64_t kCompleteCallstackId = 1;
constexpr uint64_t kInCompleteCallstackId = 2;
constexpr uint64_t kAnotherCompleteCallstackId = 3;

constexpr TimestampNs kCaptureStart(123);
constexpr RelativeTimeNs kRelativeTime1(10);
constexpr RelativeTimeNs kRelativeTime2(15);
constexpr RelativeTimeNs kRelativeTime3(20);
constexpr RelativeTimeNs kRelativeTime4(30);
constexpr RelativeTimeNs kRelativeTime5(40);
constexpr RelativeTimeNs kRelativeTimeTooLate(1000);
constexpr TID kTID{0x3AD1};
constexpr TID kAnotherTID{0x3AD2};
constexpr TID kNamelessTID{0x3AD3};
const std::string_view kThreadName = "thread";
const std::string_view kOtherThreadName = "other thread";
const absl::flat_hash_map<uint32_t, std::string> kThreadNames = {
    {*kTID, std::string(kThreadName)}, {*kAnotherTID, std::string(kOtherThreadName)}};

const absl::flat_hash_map<TID, std::string> kSampledTidToName = [] {
  absl::flat_hash_map<TID, std::string> result;
  std::transform(std::begin(kThreadNames), std::end(kThreadNames),
                 std::inserter(result, std::begin(result)), [](const auto& tid_to_name) {
                   return std::make_pair(TID(tid_to_name.first), tid_to_name.second);
                 });
  result[kNamelessTID] = "";
  return result;
}();

const absl::flat_hash_map<AbsoluteAddress, SFID> kAddressToId = {
    {kAddressFood, SFID(1)}, {kAddressCall, SFID(2)}, {kAddressBefore, SFID(3)}};

const std::unique_ptr<orbit_client_data::CallstackData> kCallstackData = [] {
  auto callstack_data = std::make_unique<orbit_client_data::CallstackData>();
  callstack_data->AddUniqueCallstack(kCompleteCallstackId, kCompleteCallstack);
  callstack_data->AddUniqueCallstack(kInCompleteCallstackId, kInCompleteCallstack);
  callstack_data->AddUniqueCallstack(kAnotherCompleteCallstackId, kAnotherCompleteCallstack);

  callstack_data->AddCallstackEvent({*kCaptureStart, kCompleteCallstackId, *kTID});
  callstack_data->AddCallstackEvent(
      {*Add(kCaptureStart, kRelativeTime1), kCompleteCallstackId, *kTID});
  callstack_data->AddCallstackEvent(
      {*Add(kCaptureStart, kRelativeTime3), kInCompleteCallstackId, *kTID});
  callstack_data->AddCallstackEvent(
      {*Add(kCaptureStart, kRelativeTime4), kAnotherCompleteCallstackId, *kAnotherTID});

  callstack_data->AddCallstackEvent(
      {*Add(kCaptureStart, kRelativeTimeTooLate), kAnotherCompleteCallstackId, *kNamelessTID});

  return callstack_data;
}();

const absl::flat_hash_map<TID, uint64_t> kTidToCallstackCount = {
    {kTID, 3}, {kAnotherTID, 1}, {kNamelessTID, 1}};

[[nodiscard]] static std::vector<SFID> SFIDsForCallstacks(const std::vector<uint64_t>& addresses) {
  std::vector<AbsoluteAddress> good_addresses;
  ForEachFrame(addresses, [&good_addresses](AbsoluteAddress address) {
    if (kAddressToId.contains(address)) {
      good_addresses.push_back(address);
    }
  });
  std::vector<SFID> ids;
  std::transform(std::begin(good_addresses), std::end(good_addresses), std::back_inserter(ids),
                 [](AbsoluteAddress address) { return kAddressToId.at(address); });
  return ids;
}

const std::vector<SFID> kCompleteCallstackIds = SFIDsForCallstacks(kCompleteCallstack.frames());
const std::vector<SFID> kInCompleteCallstackIds =
    SFIDsForCallstacks({kInCompleteCallstack.frames()[0]});
const std::vector<SFID> kAnotherCompleteCallstackIds =
    SFIDsForCallstacks(kAnotherCompleteCallstack.frames());

namespace {

const std::vector<TimestampNs> kStarts = {kCaptureStart, Add(kCaptureStart, kRelativeTime2),
                                          Add(kCaptureStart, kRelativeTime5)};

class MockFrameTrackManager {
 public:
  static inline const MockMizarData* passed_data_{};

  explicit MockFrameTrackManager(const MockMizarData* data) { passed_data_ = data; }

  [[nodiscard]] std::vector<TimestampNs> GetFrameStarts(FrameTrackId, TimestampNs,
                                                        TimestampNs) const {
    return kStarts;
  }
};

}  // namespace

constexpr RelativeTimeNs kSamplingPeriod(10);

const std::vector<ScopeId> kScopeIds = {ScopeId(1), ScopeId(2), ScopeId(10), ScopeId(30)};
const std::vector<orbit_client_data::ScopeInfo> kScopeInfos = {
    {"Foo", orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction},
    {"Bar", orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction},
    {"Manual Sync Foo", orbit_client_data::ScopeType::kApiScope},
    {"Manual Async Foo", orbit_client_data::ScopeType::kApiScopeAsync}};
const absl::flat_hash_map<ScopeId, orbit_client_data::ScopeInfo> kScopeIdToInfo =
    MakeMap(kScopeIds, kScopeInfos);
const absl::flat_hash_map<ScopeId, orbit_client_data::ScopeInfo> kFrameTracks = [] {
  absl::flat_hash_map<ScopeId, orbit_client_data::ScopeInfo> result;
  std::copy_if(std::begin(kScopeIdToInfo), std::end(kScopeIdToInfo),
               std::inserter(result, std::begin(result)), [](const auto& id_to_info) {
                 const orbit_client_data::ScopeType type = id_to_info.second.GetType();
                 return type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
                        type == orbit_client_data::ScopeType::kApiScope;
               });
  return result;
}();

using MizarPairedDataUnderTest =
    MizarPairedDataTmpl<MockMizarData, MockFrameTrackManager, MockFrameTrackStats>;

class MizarPairedDataTest : public ::testing::Test {
 public:
  MizarPairedDataTest()
      : capture_data_(std::make_unique<MockCaptureData>()),
        data_(std::make_unique<MockMizarData>()) {
    EXPECT_CALL(*capture_data_, GetCallstackData).WillRepeatedly(ReturnRef(*kCallstackData));
    EXPECT_CALL(*capture_data_, thread_names).WillRepeatedly(ReturnRef(kThreadNames));
    EXPECT_CALL(*capture_data_, GetAllProvidedScopeIds).WillRepeatedly(Return(kScopeIds));
    EXPECT_CALL(*capture_data_, GetScopeInfo).WillRepeatedly(Invoke([](const ScopeId id) {
      return kScopeIdToInfo.at(id);
    }));
    EXPECT_CALL(*data_, GetCaptureData).WillRepeatedly(ReturnRef(*capture_data_));
    EXPECT_CALL(*data_, GetCaptureStartTimestampNs).WillRepeatedly(Return(kCaptureStart));
    EXPECT_CALL(*data_, GetNominalSamplingPeriodNs).WillRepeatedly(Return(kSamplingPeriod));
  }

 protected:
  std::unique_ptr<MockCaptureData> capture_data_;
  std::unique_ptr<MockMizarData> data_;
};

TEST_F(MizarPairedDataTest, FrameTrackManagerIsProperlyInitialized) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);

  EXPECT_EQ(&MockFrameTrackManager::passed_data_->GetCaptureData(), capture_data_.get());
}

TEST_F(MizarPairedDataTest, ForeachCallstackIsCorrect) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);
  std::vector<std::vector<SFID>> actual_ids_fed_to_action;
  auto action = [&actual_ids_fed_to_action](const std::vector<SFID> ids) {
    actual_ids_fed_to_action.push_back(ids);
  };

  // all timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(kTID, RelativeTimeNs(0), kRelativeTime5, action);
  EXPECT_THAT(
      actual_ids_fed_to_action,
      UnorderedElementsAre(kCompleteCallstackIds, kCompleteCallstackIds, kInCompleteCallstackIds));

  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(kAnotherTID, RelativeTimeNs(0), kRelativeTime5, action);
  EXPECT_THAT(actual_ids_fed_to_action, UnorderedElementsAre(kAnotherCompleteCallstackIds));

  //  some timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(kTID, kRelativeTime1, kRelativeTime5, action);
  EXPECT_THAT(actual_ids_fed_to_action,
              UnorderedElementsAre(kCompleteCallstackIds, kInCompleteCallstackIds));

  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(kAnotherTID, kRelativeTime1, kRelativeTime5, action);
  EXPECT_THAT(actual_ids_fed_to_action, UnorderedElementsAre(kAnotherCompleteCallstackIds));
}

const auto kExpectedInvocationTimes = {Times(kSamplingPeriod, 2), Times(kSamplingPeriod, 2)};

TEST_F(MizarPairedDataTest, ActiveInvocationTimesIsCorrect) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);
  std::vector<RelativeTimeNs> actual_active_invocation_times =
      mizar_paired_data.ActiveInvocationTimes({kTID, kAnotherTID}, FrameTrackId(ScopeId(1)),
                                              RelativeTimeNs(0),
                                              RelativeTimeNs(std::numeric_limits<uint64_t>::max()));
  EXPECT_THAT(actual_active_invocation_times, ElementsAreArray(kExpectedInvocationTimes));
}

TEST_F(MizarPairedDataTest, ActiveInvocationTimeStats) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);
  MockFrameTrackStats stat = mizar_paired_data.ActiveInvocationTimeStats(
      {kTID, kAnotherTID}, FrameTrackId(ScopeId(1)), RelativeTimeNs(0),
      RelativeTimeNs(std::numeric_limits<uint64_t>::max()));
  EXPECT_THAT(stat.durations_fed_since_last_instantiation_,
              ElementsAreArray(kExpectedInvocationTimes));
}

TEST_F(MizarPairedDataTest, TidToNamesIsCorrect) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_THAT(mizar_paired_data.TidToNames(), UnorderedElementsAreArray(kSampledTidToName));
}

TEST_F(MizarPairedDataTest, TidToCallstackCountsIsCorrect) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_THAT(mizar_paired_data.TidToCallstackSampleCounts(),
              UnorderedElementsAreArray(kTidToCallstackCount));
}

TEST_F(MizarPairedDataTest, CaptureDurationIsCorrect) {
  MizarPairedDataUnderTest mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_EQ(mizar_paired_data.CaptureDurationNs(), kRelativeTimeTooLate);
}

}  // namespace orbit_mizar_data