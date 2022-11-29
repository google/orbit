// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>
#include <stdint.h>

#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <array>
#include <limits>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarModels/FrameTrackListModel.h"
#include "OrbitBase/Typedef.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_client_data::ScopeType;
using ::orbit_grpc_protos::PresentEvent;
using ::orbit_mizar_base::RelativeTimeNs;
using ::orbit_mizar_base::TID;
using ::orbit_mizar_data::FrameTrackId;
using ::orbit_mizar_data::FrameTrackInfo;
using ::orbit_test_utils::MakeMap;
using ::testing::AtLeast;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::UnorderedElementsAreArray;

namespace {

struct MockFrameTrackStats {
  MOCK_METHOD(uint64_t, ComputeAverageTimeNs, (), (const));
  MOCK_METHOD(uint64_t, count, (), (const));
};

struct MockPairedData {
  MOCK_METHOD((absl::flat_hash_map<FrameTrackId, FrameTrackInfo>), GetFrameTracks, (), (const));
  MOCK_METHOD((std::pair<MockFrameTrackStats, MockFrameTrackStats>&),
              WallClockAndActiveInvocationTimeStats,
              (const absl::flat_hash_set<TID>&, FrameTrackId, RelativeTimeNs, RelativeTimeNs),
              (const));
};

}  // namespace

namespace orbit_mizar_models {

const absl::flat_hash_set<TID> kTids = {TID(0x3EAD1), TID(0x3EAD2)};
constexpr RelativeTimeNs kStart(123);
constexpr RelativeTimeNs kEnd(std::numeric_limits<uint64_t>::max());

constexpr size_t kScopeFrameTracksCount = 3;
constexpr size_t kFrameTracksCount = kScopeFrameTracksCount + 1;
constexpr std::array<FrameTrackId, kFrameTracksCount> kFrameTrackIds = {
    FrameTrackId(ScopeId(1)), FrameTrackId(ScopeId(2)), FrameTrackId(ScopeId(10)),
    FrameTrackId(PresentEvent::kD3d9)};
constexpr std::array<std::string_view, kScopeFrameTracksCount> kFrameTrackNames = {"Foo", "Boo",
                                                                                   "Manual"};
constexpr std::array<ScopeType, kScopeFrameTracksCount> kScopeInfoTypes = {
    ScopeType::kDynamicallyInstrumentedFunction, ScopeType::kDynamicallyInstrumentedFunction,
    ScopeType::kApiScope};
constexpr std::array<FrameTrackId, kFrameTracksCount> kScopeIdsInExpectedOrder = {
    FrameTrackId(ScopeId(2)), FrameTrackId(ScopeId(1)), FrameTrackId(ScopeId(10)),
    FrameTrackId(PresentEvent::kD3d9)};
const std::vector<FrameTrackInfo> kFrameTrackInfos = [] {
  std::vector<FrameTrackInfo> result;
  for (size_t i = 0; i < kScopeFrameTracksCount; ++i) {
    const std::string_view name_view = kFrameTrackNames[i];
    orbit_client_data::ScopeInfo info(std::string(name_view), kScopeInfoTypes[i]);
    result.emplace_back(info);
  }
  result.emplace_back(PresentEvent::kD3d9);
  return result;
}();

const absl::flat_hash_map<FrameTrackId, FrameTrackInfo> kFrameTracks =
    MakeMap(kFrameTrackIds, kFrameTrackInfos);

constexpr std::array<std::string_view, kFrameTracksCount> kExpectedShown = {
    "[  D] Foo", "[  D] Boo", "[ MS] Manual", "[ETW] D3d9"};

const absl::flat_hash_map<FrameTrackId, std::string_view> kIdToExpectedShown =
    MakeMap(kFrameTrackIds, kExpectedShown);

TEST(FrameTrackListModelTest, NoFrameTracks) {
  MockPairedData data;

  absl::flat_hash_set<TID> tids = {};
  RelativeTimeNs start(123);

  FrameTrackListModelTmpl<MockPairedData> model(&data, &tids, &start);
  EXPECT_EQ(model.rowCount({}), 0);
}

static void SetExpectationsForMockFrameTrackStats(MockFrameTrackStats& stats) {
  EXPECT_CALL(stats, ComputeAverageTimeNs).Times(1);
  EXPECT_CALL(stats, count).Times(AtLeast(1));
}

TEST(FrameTrackListModelTest, TestDisplayTooltipAndIdRoles) {
  MockPairedData data;
  EXPECT_CALL(data, GetFrameTracks).WillRepeatedly(Return(kFrameTracks));

  FrameTrackListModelTmpl<MockPairedData> model(&data, &kTids, &kStart);

  ASSERT_EQ(model.rowCount({}), kFrameTracksCount);

  for (size_t row = 0; row < kFrameTracksCount; ++row) {
    const QModelIndex index = model.index(row);

    const auto actual_id = model.data(index, kFrameTrackIdRole).value<FrameTrackId>();
    ASSERT_EQ(actual_id, kScopeIdsInExpectedOrder[row]);

    const auto actual_shown = model.data(index, Qt::DisplayRole).value<QString>();
    EXPECT_EQ(actual_shown.toStdString(), kIdToExpectedShown.at(actual_id));

    std::pair<MockFrameTrackStats, MockFrameTrackStats> wall_clock_and_active_time_stats;
    SetExpectationsForMockFrameTrackStats(wall_clock_and_active_time_stats.first);
    SetExpectationsForMockFrameTrackStats(wall_clock_and_active_time_stats.second);

    EXPECT_CALL(data, WallClockAndActiveInvocationTimeStats)
        .WillOnce(
            Invoke([&wall_clock_and_active_time_stats, &actual_id](
                       const absl::flat_hash_set<TID>& tids, FrameTrackId id, RelativeTimeNs start,
                       RelativeTimeNs end) -> std::pair<MockFrameTrackStats, MockFrameTrackStats>& {
              EXPECT_EQ(tids, kTids);
              EXPECT_EQ(start, kStart);
              EXPECT_EQ(end, kEnd);
              EXPECT_EQ(id, actual_id);

              return wall_clock_and_active_time_stats;
            }));
    std::ignore = model.data(index, Qt::ToolTipRole);
  }
}

}  // namespace orbit_mizar_models