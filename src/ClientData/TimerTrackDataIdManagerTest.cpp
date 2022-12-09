// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <cstdint>
#include <set>
#include <string>

#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"

namespace orbit_client_data {

TEST(TimerTrackDataIdManager, SchedulerTrack) {
  TimerTrackDataIdManager timer_track_data_id_manager;
  // SchedulerTrackId should always be the same
  EXPECT_EQ(timer_track_data_id_manager.GenerateSchedulerTrackId(),
            timer_track_data_id_manager.GenerateSchedulerTrackId());
}

TEST(ThreadTrackDataProvider, ThreadTracksDifferentId) {
  TimerTrackDataIdManager timer_track_data_id_manager;
  constexpr uint32_t kThreadId1 = 42;
  constexpr uint32_t kThreadId2 = 27;

  uint32_t track_id_thread_id_1 = timer_track_data_id_manager.GenerateThreadTrackId(kThreadId1);
  uint32_t track_id_thread_id_2 = timer_track_data_id_manager.GenerateThreadTrackId(kThreadId2);

  // Only asking for the same thread_id should produce the same id.
  EXPECT_EQ(track_id_thread_id_1, timer_track_data_id_manager.GenerateThreadTrackId(kThreadId1));
  EXPECT_NE(track_id_thread_id_1, track_id_thread_id_2);
  EXPECT_NE(track_id_thread_id_1, timer_track_data_id_manager.GenerateSchedulerTrackId());
  EXPECT_NE(track_id_thread_id_2, timer_track_data_id_manager.GenerateSchedulerTrackId());
}

TEST(ThreadTrackDataProvider, DifferentTypesDifferentIds) {
  TimerTrackDataIdManager timer_track_data_id_manager;
  constexpr uint64_t kSharedId = 42;
  constexpr uint64_t kAnotherId = 43;
  const std::string async_track_name = "Example Name";

  uint32_t scheduler_track_id = timer_track_data_id_manager.GenerateSchedulerTrackId();
  uint32_t thread_track_id = timer_track_data_id_manager.GenerateThreadTrackId(kSharedId);
  uint32_t frame_track_id = timer_track_data_id_manager.GenerateFrameTrackId(kSharedId);
  uint32_t gpu_track_id = timer_track_data_id_manager.GenerateGpuTrackId(kSharedId);
  uint32_t async_track_id = timer_track_data_id_manager.GenerateAsyncTrackId(async_track_name);

  std::set<uint32_t> used_ids;
  used_ids.insert(scheduler_track_id);
  used_ids.insert(thread_track_id);
  used_ids.insert(frame_track_id);
  used_ids.insert(gpu_track_id);
  used_ids.insert(async_track_id);
  // Each id should be unique.
  EXPECT_EQ(used_ids.size(), 5);

  // The requested id should be the same when it's the same type and key.
  EXPECT_EQ(frame_track_id, timer_track_data_id_manager.GenerateFrameTrackId(kSharedId));
  EXPECT_EQ(gpu_track_id, timer_track_data_id_manager.GenerateGpuTrackId(kSharedId));
  EXPECT_NE(gpu_track_id, timer_track_data_id_manager.GenerateGpuTrackId(kAnotherId));
}

TEST(ThreadTrackDataProvider, GetTrackIdFromTimerInfo) {
  TimerTrackDataIdManager timer_track_data_id_manager;
  constexpr uint64_t kSharedId = 42;
  constexpr uint64_t kAnotherId = 43;

  std::set<uint32_t> used_tracks_ids;
  TimerInfo timer_info;

  // GetTrackId for different types of Timer Tracks.
  timer_info.set_function_id(kSharedId);
  timer_info.set_type(TimerInfo::kFrame);
  used_tracks_ids.insert(timer_track_data_id_manager.GenerateTrackIdFromTimerInfo(timer_info));

  timer_info.set_timeline_hash(kSharedId);
  timer_info.set_type(TimerInfo::kGpuCommandBuffer);
  used_tracks_ids.insert(timer_track_data_id_manager.GenerateTrackIdFromTimerInfo(timer_info));

  timer_info.set_thread_id(kSharedId);
  timer_info.set_type(TimerInfo::kNone);
  uint32_t thread_track_id = timer_track_data_id_manager.GenerateTrackIdFromTimerInfo(timer_info);
  used_tracks_ids.insert(thread_track_id);

  timer_info.set_thread_id(kAnotherId);
  used_tracks_ids.insert(timer_track_data_id_manager.GenerateTrackIdFromTimerInfo(timer_info));

  timer_info.set_thread_id(kSharedId);
  uint32_t initial_thread_track_id =
      timer_track_data_id_manager.GenerateTrackIdFromTimerInfo(timer_info);
  // This shoudln't insert a new id.
  used_tracks_ids.insert(initial_thread_track_id);

  EXPECT_EQ(thread_track_id, initial_thread_track_id);

  // 2 ThreadTracks, 1 FrameTrack, 1 GpuTrack
  EXPECT_EQ(used_tracks_ids.size(), 4);
}

}  // namespace orbit_client_data
