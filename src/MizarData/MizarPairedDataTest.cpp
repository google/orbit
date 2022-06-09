// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock-actions.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <memory>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CaptureData.h"
#include "MizarData/BaselineAndComparison.h"

namespace {

class MockCaptureData {
 public:
  MOCK_METHOD(const orbit_client_data::CallstackData&, GetCallstackData, (), (const));
};

class MockMizarData {
 public:
  MOCK_METHOD(const MockCaptureData&, GetCaptureData, (), (const));
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

constexpr uint64_t kTime0 = 123;
constexpr uint64_t kTime1 = kTime0 + 1;
constexpr uint64_t kTime2 = kTime0 + 2;
constexpr uint64_t kTime3 = kTime0 + 3;
constexpr uint64_t kTime4 = kTime0 + 4;
constexpr uint64_t kTID = 0x3AD1;
constexpr uint64_t kAnotherTID = 0x3AD2;

const absl::flat_hash_map<uint64_t, SFID> kAddressToId = {
    {kAddressFood, SFID(1)}, {kAddressCall, SFID(2)}, {kAddressBefore, SFID(3)}};

const std::unique_ptr<orbit_client_data::CallstackData> kCallstackData = [] {
  auto callstack_data = std::make_unique<orbit_client_data::CallstackData>();
  callstack_data->AddUniqueCallstack(kCompleteCallstackId, kCompleteCallstack);
  callstack_data->AddUniqueCallstack(kInCompleteCallstackId, kInCompleteCallstack);
  callstack_data->AddUniqueCallstack(kAnotherCompleteCallstackId, kAnotherCompleteCallstack);

  callstack_data->AddCallstackEvent({kTime0, kCompleteCallstackId, kTID});
  callstack_data->AddCallstackEvent({kTime1, kCompleteCallstackId, kTID});
  callstack_data->AddCallstackEvent({kTime2, kInCompleteCallstackId, kTID});
  callstack_data->AddCallstackEvent({kTime3, kAnotherCompleteCallstackId, kAnotherTID});

  return callstack_data;
}();

[[nodiscard]] static std::vector<SFID> SFIDsForCallstacks(const std::vector<uint64_t>& addresses) {
  std::vector<uint64_t> good_addresses;
  std::copy_if(std::begin(addresses), std::end(addresses), std::back_inserter(good_addresses),
               [](uint64_t address) { return kAddressToId.contains(address); });
  std::vector<SFID> ids;
  std::transform(std::begin(good_addresses), std::end(good_addresses), std::back_inserter(ids),
                 [](uint64_t address) { return SFID(kAddressToId.at(address)); });
  return ids;
}

const std::vector<SFID> kCompleteCallstackIds = SFIDsForCallstacks(kCompleteCallstack.frames());
const std::vector<SFID> kInCompleteCallstackIds =
    SFIDsForCallstacks({kInCompleteCallstack.frames()[0]});
const std::vector<SFID> kAnotherCompleteCallstackIds =
    SFIDsForCallstacks(kAnotherCompleteCallstack.frames());

TEST(MizarPairedDataTest, ForeachCallstackIsCorrect) {
  auto capture_data = std::make_unique<MockCaptureData>();
  auto data = std::make_unique<MockMizarData>();

  EXPECT_CALL(*capture_data, GetCallstackData).WillRepeatedly(testing::ReturnRef(*kCallstackData));
  EXPECT_CALL(*data, GetCaptureData).WillRepeatedly(testing::ReturnRef(*capture_data));

  MizarPairedData<MockMizarData> mizar_paired_data(std::move(data), kAddressToId);
  std::vector<std::vector<SFID>> actual_ids_fed_to_action;
  auto action = [&actual_ids_fed_to_action](const std::vector<SFID> ids) {
    actual_ids_fed_to_action.push_back(ids);
  };

  // All threads, all timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(orbit_base::kAllProcessThreadsTid, 0, kTime4, action);
  EXPECT_THAT(actual_ids_fed_to_action,
              testing::UnorderedElementsAre(kCompleteCallstackIds, kCompleteCallstackIds,
                                            kInCompleteCallstackIds, kAnotherCompleteCallstackIds));

  // One thread, all timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(kTID, 0, kTime4, action);
  EXPECT_THAT(actual_ids_fed_to_action,
              testing::UnorderedElementsAre(kCompleteCallstackIds, kCompleteCallstackIds,
                                            kInCompleteCallstackIds));

  // All threads, some timestamps
  actual_ids_fed_to_action.clear();
  mizar_paired_data.ForEachCallstackEvent(orbit_base::kAllProcessThreadsTid, kTime1, kTime4,
                                          action);
  EXPECT_THAT(actual_ids_fed_to_action,
              testing::UnorderedElementsAre(kCompleteCallstackIds, kInCompleteCallstackIds,
                                            kAnotherCompleteCallstackIds));
}

}  // namespace orbit_mizar_data