// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include <gtest/gtest.h>
#include <stddef.h>

#include <initializer_list>
#include <memory>
#include <set>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/SchedulerTrack.h"
#include "OrbitGl/StaticTimeGraphLayout.h"
#include "OrbitGl/ThreadTrack.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/TrackManager.h"
#include "OrbitGl/TrackTestData.h"

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
    timer.set_function_id(TrackTestData::kFunctionId);

    scheduler_track->OnTimer(timer);
    timer.set_type(TimerInfo::kCoreActivity);
    thread_track->OnTimer(timer);

    timer.set_thread_id(TrackTestData::kTimerOnlyThreadId);
    scheduler_track->OnTimer(timer);
    timer.set_type(TimerInfo::kCoreActivity);
    timer_only_thread_track->OnTimer(timer);
    capture_data_->UpdateScopeStats(timer);
  }

  orbit_gl::StaticTimeGraphLayout layout_;
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
  track_manager_.GetOrCreateFrameTrack(TrackTestData::kFunctionId);
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

}  // namespace orbit_gl