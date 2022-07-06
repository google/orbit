// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <array>
#include <cstdint>
#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/ScopeInfo.h"
#include "MizarBase/SampledFunctionId.h"
#include "MizarData/MizarPairedData.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_mizar_base::SFID;
using ::orbit_mizar_base::TID;
using orbit_test_utils::MakeMap;
using ::testing::ElementsAre;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAre;
using ::testing::UnorderedElementsAreArray;

namespace {

class MockCaptureData {
 public:
  MOCK_METHOD(const orbit_client_data::CallstackData&, GetCallstackData, (), (const));
  MOCK_METHOD(std::vector<const TimerInfo*>, GetTimersForScope, (uint64_t, uint64_t, uint64_t),
              (const));
  MOCK_METHOD((const absl::flat_hash_map<uint32_t, std::string>&), thread_names, (), (const));
  MOCK_METHOD(std::vector<uint64_t>, GetAllProvidedScopeIds, (), (const));
  MOCK_METHOD(orbit_client_data::ScopeInfo, GetScopeInfo, (uint64_t scope_id), (const));
};

class MockMizarData {
 public:
  MOCK_METHOD(const MockCaptureData&, GetCaptureData, (), (const));
  MOCK_METHOD(uint64_t, GetCaptureStartTimestampNs, (), (const));
  MOCK_METHOD(uint64_t, GetNominalSamplingPeriodNs, (), (const));
};

}  // namespace

namespace orbit_mizar_data {
constexpr uint64_t kAddressFood = 0xF00D;
constexpr uint64_t kAddressBad = 0xBAD;
constexpr uint64_t kAddressCall = 0xCA11;
constexpr uint64_t kAddressBefore = 0xB3F0;

const orbit_client_data::CallstackInfo kCompleteCallstack(
    {kAddressBefore, kAddressCall, kAddressBad}, orbit_client_data::CallstackType::kComplete);
const orbit_client_data::CallstackInfo kInCompleteCallstack(
    {kAddressBefore, kAddressCall, kAddressBad},
    orbit_client_data::CallstackType::kDwarfUnwindingError);
const orbit_client_data::CallstackInfo kAnotherCompleteCallstack(
    {kAddressBefore, kAddressCall, kAddressFood}, orbit_client_data::CallstackType::kComplete);

constexpr uint64_t kCompleteCallstackId = 1;
constexpr uint64_t kInCompleteCallstackId = 2;
constexpr uint64_t kAnotherCompleteCallstackId = 3;

constexpr uint64_t kCaptureStart = 123;
constexpr uint64_t kRelativeTime1 = 10;
constexpr uint64_t kRelativeTime2 = 15;
constexpr uint64_t kRelativeTime3 = 20;
constexpr uint64_t kRelativeTime4 = 30;
constexpr uint64_t kRelativeTime5 = 40;
constexpr uint64_t kRelativeTimeTooLate = 1000;
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

const absl::flat_hash_map<uint64_t, SFID> kAddressToId = {
    {kAddressFood, SFID(1)}, {kAddressCall, SFID(2)}, {kAddressBefore, SFID(3)}};

const std::unique_ptr<orbit_client_data::CallstackData> kCallstackData = [] {
  auto callstack_data = std::make_unique<orbit_client_data::CallstackData>();
  callstack_data->AddUniqueCallstack(kCompleteCallstackId, kCompleteCallstack);
  callstack_data->AddUniqueCallstack(kInCompleteCallstackId, kInCompleteCallstack);
  callstack_data->AddUniqueCallstack(kAnotherCompleteCallstackId, kAnotherCompleteCallstack);

  callstack_data->AddCallstackEvent({kCaptureStart, kCompleteCallstackId, *kTID});
  callstack_data->AddCallstackEvent({kCaptureStart + kRelativeTime1, kCompleteCallstackId, *kTID});
  callstack_data->AddCallstackEvent(
      {kCaptureStart + kRelativeTime3, kInCompleteCallstackId, *kTID});
  callstack_data->AddCallstackEvent(
      {kCaptureStart + kRelativeTime4, kAnotherCompleteCallstackId, *kAnotherTID});

  callstack_data->AddCallstackEvent(
      {kCaptureStart + kRelativeTimeTooLate, kAnotherCompleteCallstackId, *kNamelessTID});

  return callstack_data;
}();

const absl::flat_hash_map<TID, uint64_t> kTidToCallstackCount = {
    {kTID, 3}, {kAnotherTID, 1}, {kNamelessTID, 1}};

[[nodiscard]] static std::vector<SFID> SFIDsForCallstacks(const std::vector<uint64_t>& addresses) {
  std::vector<uint64_t> good_addresses;
  std::copy_if(std::begin(addresses), std::end(addresses), std::back_inserter(good_addresses),
               [](uint64_t address) { return kAddressToId.contains(address); });
  std::vector<SFID> ids;
  std::transform(std::begin(good_addresses), std::end(good_addresses), std::back_inserter(ids),
                 [](uint64_t address) { return kAddressToId.at(address); });
  return ids;
}

const std::vector<SFID> kCompleteCallstackIds = SFIDsForCallstacks(kCompleteCallstack.frames());
const std::vector<SFID> kInCompleteCallstackIds =
    SFIDsForCallstacks({kInCompleteCallstack.frames()[0]});
const std::vector<SFID> kAnotherCompleteCallstackIds =
    SFIDsForCallstacks(kAnotherCompleteCallstack.frames());

constexpr size_t kInvocationsCount = 3;
constexpr std::array<uint64_t, kInvocationsCount> kStarts = {
    kCaptureStart, kCaptureStart + kRelativeTime2, kCaptureStart + kRelativeTime5};
const std::array<uint64_t, kInvocationsCount> kEnds = [] {
  std::array<uint64_t, kInvocationsCount> result{};
  std::transform(std::begin(kStarts), std::end(kStarts), std::begin(result),
                 [](const uint64_t start) { return start + 1; });
  return result;
}();

const std::array<orbit_client_protos::TimerInfo, kInvocationsCount> kTimers = [] {
  std::array<orbit_client_protos::TimerInfo, kInvocationsCount> result;
  std::transform(std::begin(kStarts), std::end(kStarts), std::begin(kEnds), std::begin(result),
                 [](const uint64_t start, const uint64_t end) {
                   orbit_client_protos::TimerInfo timer;
                   timer.set_start(start);
                   timer.set_end(end);
                   timer.set_thread_id(*kTID);
                   return timer;
                 });
  return result;
}();

const std::vector<const orbit_client_protos::TimerInfo*> kTimerPtrs = [] {
  std::vector<const orbit_client_protos::TimerInfo*> result;
  std::transform(std::begin(kTimers), std::end(kTimers), std::back_inserter(result),
                 [](const TimerInfo& timer) { return &timer; });
  return result;
}();

constexpr uint64_t kSamplingPeriod = 10;

const std::vector<uint64_t> kScopeIds = {1, 2, 10, 30};
const std::vector<orbit_client_data::ScopeInfo> kScopeInfos = {
    {"Foo", orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction},
    {"Bar", orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction},
    {"Manual Sync Foo", orbit_client_data::ScopeType::kApiScope},
    {"Manual Async Foo", orbit_client_data::ScopeType::kApiScopeAsync}};
const absl::flat_hash_map<uint64_t, orbit_client_data::ScopeInfo> kScopeIdToInfo =
    MakeMap(kScopeIds, kScopeInfos);
const absl::flat_hash_map<uint64_t, orbit_client_data::ScopeInfo> kFrameTracks = [] {
  absl::flat_hash_map<uint64_t, orbit_client_data::ScopeInfo> result;
  std::copy_if(std::begin(kScopeIdToInfo), std::end(kScopeIdToInfo),
               std::inserter(result, std::begin(result)), [](const auto& id_to_info) {
                 const orbit_client_data::ScopeType type = id_to_info.second.GetType();
                 return type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
                        type == orbit_client_data::ScopeType::kApiScope;
               });
  return result;
}();

class MizarPairedDataTest : public ::testing::Test {
 public:
  MizarPairedDataTest()
      : capture_data_(std::make_unique<MockCaptureData>()),
        data_(std::make_unique<MockMizarData>()) {
    EXPECT_CALL(*capture_data_, GetCallstackData).WillRepeatedly(ReturnRef(*kCallstackData));
    EXPECT_CALL(*capture_data_, GetTimersForScope).WillRepeatedly(Return(kTimerPtrs));
    EXPECT_CALL(*capture_data_, thread_names).WillRepeatedly(ReturnRef(kThreadNames));
    EXPECT_CALL(*capture_data_, GetAllProvidedScopeIds).WillRepeatedly(Return(kScopeIds));
    EXPECT_CALL(*capture_data_, GetScopeInfo).WillRepeatedly(Invoke([](const uint64_t id) {
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

TEST_F(MizarPairedDataTest, ForeachCallstackIsCorrect) {
  MizarPairedDataTmpl<MockMizarData> mizar_paired_data(std::move(data_), kAddressToId);
  std::vector<std::vector<SFID>> actual_ids_fed_to_action;
  auto action = [&actual_ids_fed_to_action](const std::vector<SFID> ids) {
    actual_ids_fed_to_action.push_back(ids);
  };

  // All threads, all timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(TID(orbit_base::kAllProcessThreadsTid), 0, kRelativeTime5,
                                          action);
  EXPECT_THAT(actual_ids_fed_to_action,
              UnorderedElementsAre(kCompleteCallstackIds, kCompleteCallstackIds,
                                   kInCompleteCallstackIds, kAnotherCompleteCallstackIds));

  // One thread, all timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(kTID, 0, kRelativeTime5, action);
  EXPECT_THAT(
      actual_ids_fed_to_action,
      UnorderedElementsAre(kCompleteCallstackIds, kCompleteCallstackIds, kInCompleteCallstackIds));

  // All threads, some timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(TID(orbit_base::kAllProcessThreadsTid), kRelativeTime1,
                                          kRelativeTime5, action);
  EXPECT_THAT(actual_ids_fed_to_action,
              UnorderedElementsAre(kCompleteCallstackIds, kInCompleteCallstackIds,
                                   kAnotherCompleteCallstackIds));
}

TEST_F(MizarPairedDataTest, ActiveInvocationTimesIsCorrect) {
  MizarPairedDataTmpl<MockMizarData> mizar_paired_data(std::move(data_), kAddressToId);
  std::vector<uint64_t> actual_active_invocation_times = mizar_paired_data.ActiveInvocationTimes(
      {kTID, kAnotherTID}, 1, 0, std::numeric_limits<uint64_t>::max());
  EXPECT_THAT(actual_active_invocation_times,
              ElementsAre(kSamplingPeriod * 2, kSamplingPeriod * 2));
}

TEST_F(MizarPairedDataTest, TidToNamesIsCorrect) {
  MizarPairedDataTmpl<MockMizarData> mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_THAT(mizar_paired_data.TidToNames(), UnorderedElementsAreArray(kSampledTidToName));
}

TEST_F(MizarPairedDataTest, TidToCallstackCountsIsCorrect) {
  MizarPairedDataTmpl<MockMizarData> mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_THAT(mizar_paired_data.TidToCallstackSampleCounts(),
              UnorderedElementsAreArray(kTidToCallstackCount));
}

TEST_F(MizarPairedDataTest, GetFrameTracksIsCorrect) {
  MizarPairedDataTmpl<MockMizarData> mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_THAT(mizar_paired_data.GetFrameTracks(), UnorderedElementsAreArray(kFrameTracks));
}

TEST_F(MizarPairedDataTest, CaptureDurationIsCorrect) {
  MizarPairedDataTmpl<MockMizarData> mizar_paired_data(std::move(data_), kAddressToId);
  EXPECT_EQ(mizar_paired_data.CaptureDurationNs(), kRelativeTimeTooLate);
}

}  // namespace orbit_mizar_data