// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/Logging.h"
#include "TimeGraph.h"
#include "Track.h"
#include "TrackManager.h"
#include "TrackTestData.h"

using orbit_client_protos::TimerInfo;

namespace orbit_gl {

const size_t kNumTracks = 3;
const size_t kNumThreadTracks = 2;
const size_t kNumSchedulerTracks = 1;

class TrackManagerTest : public ::testing::Test {
 public:
  explicit TrackManagerTest()
      : capture_data_(TrackTestData::GenerateTestCaptureData()),
        track_manager_(nullptr, nullptr, nullptr, &layout_, nullptr, nullptr, capture_data_.get()) {
  }

 protected:
  void CreateAndFillTracks() {
    auto* scheduler_track = track_manager_.GetOrCreateSchedulerTrack();
    auto* thread_track = track_manager_.GetOrCreateThreadTrack(TrackTestData::kThreadId);
    auto* timer_only_thread_track =
        track_manager_.GetOrCreateThreadTrack(TrackTestData::kTimerOnlyThreadId);

    TimerInfo timer;
    timer.set_start(0);
    timer.set_end(100);
    timer.set_thread_id(TrackTestData::kThreadId);
    timer.set_processor(0);
    timer.set_depth(0);
    timer.set_type(TimerInfo::kCoreActivity);

    scheduler_track->OnTimer(timer);
    timer.set_type(TimerInfo::kCoreActivity);
    thread_track->OnTimer(timer);

    timer.set_thread_id(TrackTestData::kTimerOnlyThreadId);
    scheduler_track->OnTimer(timer);
    timer.set_type(TimerInfo::kCoreActivity);
    timer_only_thread_track->OnTimer(timer);
  }

  TimeGraphLayout layout_;
  std::unique_ptr<orbit_client_data::CaptureData> capture_data_;
  TrackManager track_manager_;
};

TEST_F(TrackManagerTest, GetOrCreateCreatesTracks) {
  EXPECT_EQ(0ull, track_manager_.GetAllTracks().size());

  track_manager_.GetOrCreateSchedulerTrack();
  EXPECT_EQ(1ull, track_manager_.GetAllTracks().size());
  track_manager_.GetOrCreateThreadTrack(42);
  EXPECT_EQ(2ull, track_manager_.GetAllTracks().size());
}

TEST_F(TrackManagerTest, FrameTracksAreReportedWithAllTracks) {
  EXPECT_EQ(0ull, track_manager_.GetAllTracks().size());

  track_manager_.GetOrCreateSchedulerTrack();
  EXPECT_EQ(1ull, track_manager_.GetAllTracks().size());
  orbit_grpc_protos::InstrumentedFunction function;
  track_manager_.GetOrCreateFrameTrack(function);
  EXPECT_EQ(2ull, track_manager_.GetAllTracks().size());
}

TEST_F(TrackManagerTest, AllButEmptyTracksAreVisible) {
  CreateAndFillTracks();
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks, track_manager_.GetVisibleTracks().size());
}

// TODO(b/181671054): Once the scheduler track stays visible, this needs to be adjusted
TEST_F(TrackManagerTest, SimpleFiltering) {
  CreateAndFillTracks();
  track_manager_.SetFilter("example");
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(1ull, track_manager_.GetVisibleTracks().size());

  track_manager_.SetFilter("thread");
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumThreadTracks, track_manager_.GetVisibleTracks().size());

  track_manager_.SetFilter("nonsense");
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(0ull, track_manager_.GetVisibleTracks().size());
}

TEST_F(TrackManagerTest, NonVisibleTracksAreNotInTheList) {
  CreateAndFillTracks();
  track_manager_.GetOrCreateSchedulerTrack()->SetVisible(false);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks - 1, track_manager_.GetVisibleTracks().size());
}

TEST_F(TrackManagerTest, FiltersAndTrackVisibilityWorkTogether) {
  CreateAndFillTracks();
  track_manager_.GetOrCreateThreadTrack(TrackTestData::kThreadId)->SetVisible(false);
  track_manager_.SetFilter("thread");
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumThreadTracks - 1, track_manager_.GetVisibleTracks().size());
}

TEST_F(TrackManagerTest, FiltersAndTrackTypeVisibilityWorkTogether) {
  CreateAndFillTracks();
  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, false);
  track_manager_.SetFilter("thread");
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(0, track_manager_.GetVisibleTracks().size());

  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, true);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumThreadTracks, track_manager_.GetVisibleTracks().size());

  track_manager_.SetFilter("");
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks, track_manager_.GetVisibleTracks().size());

  track_manager_.SetFilter("thread");
  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, false);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(0, track_manager_.GetVisibleTracks().size());
}

TEST_F(TrackManagerTest, TrackTypeVisibilityAffectsVisibleTrackList) {
  CreateAndFillTracks();

  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, false);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks - kNumThreadTracks, track_manager_.GetVisibleTracks().size());

  track_manager_.SetTrackTypeVisibility(Track::Type::kSchedulerTrack, false);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks - kNumThreadTracks - kNumSchedulerTracks,
            track_manager_.GetVisibleTracks().size());
  track_manager_.SetTrackTypeVisibility(Track::Type::kSchedulerTrack, true);

  track_manager_.GetOrCreateThreadTrack(TrackTestData::kThreadId)->SetVisible(false);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks - kNumThreadTracks, track_manager_.GetVisibleTracks().size());

  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, true);
  track_manager_.UpdateTrackListForRendering();
  EXPECT_EQ(kNumTracks - 1, track_manager_.GetVisibleTracks().size());
}

TEST_F(TrackManagerTest, TrackTypeVisibilityIsRestored) {
  CreateAndFillTracks();
  auto match_visible_track_types = [&](std::set<Track::Type> types) -> bool {
    bool result = true;

    for (Track::Type type : Track::kAllTrackTypes) {
      auto type_it = types.find(type);
      if (type_it != types.end()) {
        result = result && track_manager_.GetTrackTypeVisibility(type);
      } else {
        result = result && !track_manager_.GetTrackTypeVisibility(type);
      }
    }

    return result;
  };

  EXPECT_TRUE(match_visible_track_types(Track::kAllTrackTypes));
  auto visibility_prev = track_manager_.GetAllTrackTypesVisibility();

  track_manager_.SetTrackTypeVisibility(Track::Type::kThreadTrack, false);
  track_manager_.SetTrackTypeVisibility(Track::Type::kSchedulerTrack, false);

  std::set<Track::Type> visible_tracks{Track::kAllTrackTypes};
  visible_tracks.erase(Track::Type::kThreadTrack);
  visible_tracks.erase(Track::Type::kSchedulerTrack);

  EXPECT_TRUE(match_visible_track_types(visible_tracks));
  auto visibility_after = track_manager_.GetAllTrackTypesVisibility();

  track_manager_.RestoreAllTrackTypesVisibility(visibility_prev);
  EXPECT_TRUE(match_visible_track_types(Track::kAllTrackTypes));

  track_manager_.RestoreAllTrackTypesVisibility(visibility_after);
  EXPECT_TRUE(match_visible_track_types(visible_tracks));
}

constexpr size_t kTimersForFirstId = 3;
constexpr size_t kTimersForSecondId = 2;
constexpr size_t kTimerCount = kTimersForFirstId + kTimersForSecondId;
constexpr uint64_t kFirstId = 1;
constexpr uint64_t kSecondId = 2;
constexpr std::array<uint64_t, kTimerCount> kTimerIds = {kFirstId, kFirstId, kFirstId, kSecondId,
                                                         kSecondId};
constexpr std::array<uint64_t, kTimerCount> kStarts = {10, 20, 30, 40, 50};
constexpr std::array<uint64_t, kTimersForFirstId> kDurationsForFirstId = {300, 100, 200};
constexpr std::array<uint64_t, kTimersForFirstId> kSortedDurationsForFirstId = {100, 200, 300};
constexpr std::array<uint64_t, kTimersForSecondId> kDurationsForSecondId = {500, 400};
constexpr std::array<uint64_t, kTimersForSecondId> kSortedDurationsForSecondId = {400, 500};

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

TEST_F(TrackManagerTest, UpdateTimerDurationsIsCorrect) {
  for (const TimerInfo& timer : kTimerInfos) {
    if (timer.function_id() == kFirstId) {
      capture_data_->GetThreadTrackDataProvider()->AddTimer(timer);
    }
  }

  auto* async_track = track_manager_.GetOrCreateAsyncTrack(TrackTestData::kAsyncTrackName);

  for (const TimerInfo& timer : kTimerInfos) {
    if (timer.function_id() == kSecondId) {
      async_track->OnTimer(timer);
    }
  }

  track_manager_.OnCaptureComplete();

  const std::vector<uint64_t>* durations_first =
      track_manager_.GetSortedTimerDurationsForScopeId(kFirstId);
  EXPECT_EQ(*durations_first,
            std::vector(std::begin(kSortedDurationsForFirstId), std::end(kSortedDurationsForFirstId)));

  const std::vector<uint64_t>* durations_second =
      track_manager_.GetSortedTimerDurationsForScopeId(kSecondId);
  EXPECT_EQ(*durations_second,
            std::vector(std::begin(kSortedDurationsForSecondId), std::end(kSortedDurationsForSecondId)));
}

}  // namespace orbit_gl