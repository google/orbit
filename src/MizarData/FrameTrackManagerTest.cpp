// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/algorithm/container.h>
#include <absl/container/flat_hash_map.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/FrameTrackManager.h"
#include "OrbitBase/Overloaded.h"
#include "OrbitBase/Typedef.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_client_data::ScopeInfo;
using ::orbit_client_protos::TimerInfo;
using ::orbit_grpc_protos::PresentEvent;
using ::orbit_mizar_base::TimestampNs;
using ::orbit_test_utils::MakeMap;
using ::testing::ElementsAreArray;
using ::testing::Invoke;
using ::testing::IsEmpty;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAreArray;

namespace {

class MockCaptureData {
 public:
  MOCK_METHOD(std::vector<const TimerInfo*>, GetTimersForScope, (ScopeId, uint64_t, uint64_t),
              (const));
  MOCK_METHOD(std::vector<ScopeId>, GetAllProvidedScopeIds, (), (const));
  MOCK_METHOD(ScopeInfo, GetScopeInfo, (ScopeId scope_id), (const));
};

class MockMizarData {
 public:
  MOCK_METHOD(const MockCaptureData&, GetCaptureData, (), (const));
  MOCK_METHOD((absl::flat_hash_map<PresentEvent::Source, std::vector<PresentEvent>>),
              source_to_present_events, (), (const));
};

}  // namespace

namespace orbit_mizar_data {

constexpr ScopeId kFirstScopeId(1);
constexpr ScopeId kSecondScopeId(2);
static const std::vector<ScopeId> kScopeIds = {kFirstScopeId, kSecondScopeId};
static const std::vector<orbit_client_data::ScopeInfo> kScopeInfos = {
    {"Foo", orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction},
    {"Bar", orbit_client_data::ScopeType::kApiScope}};

static std::vector<TimestampNs> MakeTimestamps(absl::Span<const uint64_t> raw) {
  std::vector<TimestampNs> result;
  absl::c_transform(raw, std::back_inserter(result),
                    [](uint64_t value) { return TimestampNs(value); });
  return result;
}

static const std::vector<TimestampNs> kFirstScopeStarts = MakeTimestamps({20, 10});
static const std::vector<TimestampNs> kSecondScopeStarts = MakeTimestamps({200, 100, 300});
static const std::vector<std::vector<TimestampNs>> kScopeFrameTrackStartLists = {
    kFirstScopeStarts, kSecondScopeStarts};

// Re-wrap the values form `TimestampNs` into `TimerInfos`
static std::vector<TimerInfo> ToTimerInfos(absl::Span<const TimestampNs> starts) {
  std::vector<TimerInfo> result;
  std::transform(std::begin(starts), std::end(starts), std::back_inserter(result),
                 [](TimestampNs start) {
                   TimerInfo timer;
                   timer.set_start(*start);
                   return timer;
                 });
  return result;
}

static const std::vector<TimerInfo> kFirstScopeTimers = ToTimerInfos(kFirstScopeStarts);
static const std::vector<TimerInfo> kSecondScopeTimers = ToTimerInfos(kSecondScopeStarts);

static std::vector<const TimerInfo*> MakePtrs(absl::Span<const TimerInfo> timers) {
  std::vector<const TimerInfo*> result;
  std::transform(std::begin(timers), std::end(timers), std::back_inserter(result),
                 [](const TimerInfo& timer) { return &timer; });
  return result;
}

static const std::vector<const TimerInfo*> kFirstScopeTimerPtrs = MakePtrs(kFirstScopeTimers);
static const std::vector<const TimerInfo*> kSecondScopeTimerPtrs = MakePtrs(kSecondScopeTimers);

static const absl::flat_hash_map<ScopeId, std::vector<const TimerInfo*>> kScopeIdToSTimerInfoPtrs =
    MakeMap(kScopeIds, std::vector<std::vector<const TimerInfo*>>{kFirstScopeTimerPtrs,
                                                                  kSecondScopeTimerPtrs});
static const absl::flat_hash_map<ScopeId, ScopeInfo> kScopeIdToInfo =
    MakeMap(kScopeIds, kScopeInfos);
static const absl::flat_hash_map<ScopeInfo, std::vector<TimestampNs>> kScopeInfoToFrameStarts =
    MakeMap(kScopeInfos, kScopeFrameTrackStartLists);

static const std::vector<TimestampNs> kDxgiFrameStarts = MakeTimestamps({10, 1, 2, 4, 20});
static const std::vector<TimestampNs> kD3d9FrameStarts = MakeTimestamps({100, 10, 20, 40, 200});

static std::vector<PresentEvent> MakePresentEvent(absl::Span<const TimestampNs> starts) {
  std::vector<PresentEvent> result;
  std::transform(std::begin(starts), std::end(starts), std::back_inserter(result),
                 [](const TimestampNs start) {
                   PresentEvent event;
                   event.set_begin_timestamp_ns(*start);
                   return event;
                 });
  return result;
}

static const std::vector<PresentEvent::Source> kEtwSources = {PresentEvent::kDxgi,
                                                              PresentEvent::kD3d9};

const absl::flat_hash_map<PresentEvent::Source, std::vector<TimestampNs>> kEtwSourceToFrameStart =
    MakeMap(kEtwSources, std::vector<std::vector<TimestampNs>>{kDxgiFrameStarts, kD3d9FrameStarts});
const absl::flat_hash_map<PresentEvent::Source, std::vector<PresentEvent>>
    kEtwSourceToPresentEvent = MakeMap(
        kEtwSources, std::vector<std::vector<PresentEvent>>{MakePresentEvent(kDxgiFrameStarts),
                                                            MakePresentEvent(kD3d9FrameStarts)});

static std::pair<std::vector<ScopeInfo>, std::vector<PresentEvent::Source>> DecomposeSources(
    const absl::flat_hash_map<FrameTrackId, FrameTrackInfo>& id_to_infos) {
  std::vector<ScopeInfo> scope_id_infos;
  std::vector<PresentEvent::Source> etw_sources;

  for (const auto& [unused_id, info] : id_to_infos) {
    std::visit(orbit_base::overloaded{
                   [&scope_id_infos](const ScopeInfo& info) { scope_id_infos.push_back(info); },
                   [&etw_sources](PresentEvent::Source source) { etw_sources.push_back(source); }},
               *info);
  }

  return {scope_id_infos, etw_sources};
}

class ExpectFrameTracksHasFrameStartsForScopes : public ::testing::Test {
 public:
  ExpectFrameTracksHasFrameStartsForScopes() {
    EXPECT_CALL(data_, GetCaptureData).WillRepeatedly(ReturnRef(capture_data_));
  }

  void SourceToPresentEventExpectCall() {
    EXPECT_CALL(data_, source_to_present_events).WillRepeatedly(Return(kEtwSourceToPresentEvent));
  }

  void CaptureDataExpectCalls() {
    EXPECT_CALL(capture_data_, GetAllProvidedScopeIds).WillRepeatedly(Return(kScopeIds));
    EXPECT_CALL(capture_data_, GetScopeInfo).WillRepeatedly(Invoke([](const ScopeId id) {
      return kScopeIdToInfo.at(id);
    }));
    EXPECT_CALL(capture_data_, GetTimersForScope)
        .WillRepeatedly(Invoke([](const ScopeId id, uint64_t /*min*/, uint64_t /*max*/) {
          return kScopeIdToSTimerInfoPtrs.at(id);
        }));
  }

  void ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs min_start,
                                                                 TimestampNs max_start) {
    for (const auto& [id, info] : frame_track_manager_.GetFrameTracks()) {
      std::vector<TimestampNs> expected_frame_starts = std::visit(
          orbit_base::overloaded{
              [](const ScopeInfo& info) { return kScopeInfoToFrameStarts.at(info); },
              [min_start, max_start](PresentEvent::Source source) {
                std::vector<TimestampNs> filtered_time_list;
                const std::vector<TimestampNs> all_frame_starts = kEtwSourceToFrameStart.at(source);
                std::copy_if(std::begin(all_frame_starts), std::end(all_frame_starts),
                             std::back_inserter(filtered_time_list),
                             [min_start, max_start](TimestampNs start) {
                               return min_start <= start && start <= max_start;
                             });
                return filtered_time_list;
              }},
          *info);
      absl::c_sort(expected_frame_starts);
      const std::vector<TimestampNs> actual_frame_starts =
          frame_track_manager_.GetFrameStarts(id, min_start, max_start);
      EXPECT_THAT(actual_frame_starts, ElementsAreArray(expected_frame_starts));
    }
  }

  void ExpectGetFrameTracksIsCorrect(
      absl::Span<const orbit_client_data::ScopeInfo> expected_scope_info,
      absl::Span<const PresentEvent::Source> expected_etw_sources) {
    const auto [scope_infos, etw_sources] = DecomposeSources(frame_track_manager_.GetFrameTracks());

    EXPECT_THAT(scope_infos, UnorderedElementsAreArray(expected_scope_info));
    EXPECT_THAT(etw_sources, UnorderedElementsAreArray(expected_etw_sources));
  }

 protected:
  MockCaptureData capture_data_;
  MockMizarData data_;
  FrameTrackManagerTmpl<MockMizarData> frame_track_manager_{&data_};
};

TEST_F(ExpectFrameTracksHasFrameStartsForScopes, FrameTracksAreCorrectForNoScopesAndNoEtws) {
  const absl::flat_hash_map<FrameTrackId, FrameTrackInfo> id_to_infos =
      frame_track_manager_.GetFrameTracks();

  EXPECT_THAT(id_to_infos, IsEmpty());
}

TEST_F(ExpectFrameTracksHasFrameStartsForScopes, FrameTracksAreCorrectForScopesAndNoEtws) {
  CaptureDataExpectCalls();

  ExpectGetFrameTracksIsCorrect(kScopeInfos, {});

  // Filtering of scopes w.r.t min/max timestamp is handles by Capture data, it is not covered
  // in the test, hence we just pass zeroes. The MockCaptureData ignores these anyway.
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(0), TimestampNs(0));
}

TEST_F(ExpectFrameTracksHasFrameStartsForScopes, FrameTracksAreCorrectForEtwsAndNoScopes) {
  SourceToPresentEventExpectCall();

  ExpectGetFrameTracksIsCorrect({}, kEtwSources);

  // The arguments are chosen w.r.t the values used in kFirstScopeStarts, kSecondScopeStarts,
  // kDxgiFrameStarts and kD3d9FrameStarts
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(0), TimestampNs(15));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(0), TimestampNs(50));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(0), TimestampNs(300));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(3), TimestampNs(15));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(15), TimestampNs(50));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(50), TimestampNs(300));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(50), TimestampNs(3000));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(1000), TimestampNs(3000));
}

TEST_F(ExpectFrameTracksHasFrameStartsForScopes, FrameTracksAreCorrectForEtwsAndScopes) {
  CaptureDataExpectCalls();
  SourceToPresentEventExpectCall();

  ExpectGetFrameTracksIsCorrect(kScopeInfos, kEtwSources);

  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(TimestampNs(0), TimestampNs(300));
}

}  // namespace orbit_mizar_data