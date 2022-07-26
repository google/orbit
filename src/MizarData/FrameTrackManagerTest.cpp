// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/FrameTrackManager.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_client_data::ScopeInfo;
using ::orbit_client_protos::TimerInfo;
using ::orbit_grpc_protos::PresentEvent;
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

static const std::vector<FrameStartNs> kFirstScopeStarts = {FrameStartNs(10), FrameStartNs(20)};
static const std::vector<FrameStartNs> kSecondScopeStarts = {FrameStartNs(100), FrameStartNs(200),
                                                             FrameStartNs(300)};
static const std::vector<std::vector<FrameStartNs>> kScopeFrameTrackStartLists = {
    kFirstScopeStarts, kSecondScopeStarts};

// Re-wrap the values form `FrameStartNs` into `TimerInfos`
static std::vector<TimerInfo> ToTimerInfos(const std::vector<FrameStartNs>& starts) {
  std::vector<TimerInfo> result;
  std::transform(std::begin(starts), std::end(starts), std::back_inserter(result),
                 [](FrameStartNs start) {
                   TimerInfo timer;
                   timer.set_start(*start);
                   return timer;
                 });
  return result;
}

static const std::vector<TimerInfo> kFirstScopeTimers = ToTimerInfos(kFirstScopeStarts);
static const std::vector<TimerInfo> kSecondScopeTimers = ToTimerInfos(kSecondScopeStarts);

static std::vector<const TimerInfo*> MakePtrs(const std::vector<TimerInfo>& timers) {
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
static const absl::flat_hash_map<ScopeInfo, std::vector<FrameStartNs>> kScopeInfoToFrameStarts =
    MakeMap(kScopeInfos, kScopeFrameTrackStartLists);

static const std::vector<FrameStartNs> kDxgiFrameStarts = {
    FrameStartNs(1), FrameStartNs(2), FrameStartNs(4), FrameStartNs(10), FrameStartNs(20)};
static const std::vector<FrameStartNs> kD3d9FrameStarts = {
    FrameStartNs(10), FrameStartNs(20), FrameStartNs(40), FrameStartNs(100), FrameStartNs(200)};

static std::vector<PresentEvent> MakePresentEvent(const std::vector<FrameStartNs>& starts) {
  std::vector<PresentEvent> result;
  std::transform(std::begin(starts), std::end(starts), std::back_inserter(result),
                 [](const FrameStartNs start) {
                   PresentEvent event;
                   event.set_begin_timestamp_ns(*start);
                   return event;
                 });
  return result;
}

static const std::vector<PresentEvent::Source> kEtwSources = {PresentEvent::kDxgi,
                                                              PresentEvent::kD3d9};

const absl::flat_hash_map<PresentEvent::Source, std::vector<FrameStartNs>> kEtwSourceToFrameStart =
    MakeMap(kEtwSources,
            std::vector<std::vector<FrameStartNs>>{kDxgiFrameStarts, kD3d9FrameStarts});
const absl::flat_hash_map<PresentEvent::Source, std::vector<PresentEvent>>
    kEtwSourceToPresentEvent = MakeMap(
        kEtwSources, std::vector<std::vector<PresentEvent>>{MakePresentEvent(kDxgiFrameStarts),
                                                            MakePresentEvent(kD3d9FrameStarts)});

static std::pair<std::vector<ScopeInfo>, std::vector<PresentEvent::Source>> DecomposeSources(
    const absl::flat_hash_map<FrameTrackId, FrameTrackInfo>& id_to_infos) {
  std::vector<ScopeInfo> scope_id_infos;
  std::vector<PresentEvent::Source> etw_sources;

  for (const auto& [unused_id, info] : id_to_infos) {
    Visit([&scope_id_infos](const ScopeInfo& info) { scope_id_infos.push_back(info); },
          [&etw_sources](PresentEvent::Source source) { etw_sources.push_back(source); }, info);
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

  void ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs min_start,
                                                                 FrameStartNs max_start) {
    for (const auto& [id, info] : frame_track_manager_.GetFrameTracks()) {
      const std::vector<FrameStartNs> expected_frame_starts = Visit(
          [](const ScopeInfo& info) { return kScopeInfoToFrameStarts.at(info); },
          [min_start, max_start](PresentEvent::Source source) {
            std::vector<FrameStartNs> filtered_time_list;
            const std::vector<FrameStartNs> all_frame_starts = kEtwSourceToFrameStart.at(source);
            std::copy_if(std::begin(all_frame_starts), std::end(all_frame_starts),
                         std::back_inserter(filtered_time_list),
                         [min_start, max_start](FrameStartNs start) {
                           return min_start <= start && start <= max_start;
                         });
            return filtered_time_list;
          },
          info);

      const std::vector<FrameStartNs> actual_frame_starts =
          frame_track_manager_.GetFrameStarts(id, min_start, max_start);
      EXPECT_THAT(actual_frame_starts, ElementsAreArray(expected_frame_starts));
    }
  }

  void ExpectGetFrameTracksIsCorrect(
      const std::vector<orbit_client_data::ScopeInfo>& expected_scope_info,
      const std::vector<PresentEvent::Source>& expected_etw_sources) {
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
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(0), FrameStartNs(0));
}

TEST_F(ExpectFrameTracksHasFrameStartsForScopes, FrameTracksAreCorrectForEtwsAndNoScopes) {
  SourceToPresentEventExpectCall();

  ExpectGetFrameTracksIsCorrect({}, kEtwSources);

  // The arguments are chosen w.r.t the values used in kFirstScopeStarts, kSecondScopeStarts,
  // kDxgiFrameStarts and kD3d9FrameStarts
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(0), FrameStartNs(15));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(0), FrameStartNs(50));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(0), FrameStartNs(300));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(3), FrameStartNs(15));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(15), FrameStartNs(50));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(50), FrameStartNs(300));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(50), FrameStartNs(3000));
  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(1000), FrameStartNs(3000));
}

TEST_F(ExpectFrameTracksHasFrameStartsForScopes, FrameTracksAreCorrectForEtwsAndScopes) {
  CaptureDataExpectCalls();
  SourceToPresentEventExpectCall();

  ExpectGetFrameTracksIsCorrect(kScopeInfos, kEtwSources);

  ExpectGetFrameTracksReturnsExpectedValueForEachFrameTrack(FrameStartNs(0), FrameStartNs(300));
}

}  // namespace orbit_mizar_data