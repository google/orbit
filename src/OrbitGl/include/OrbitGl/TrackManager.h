// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_MANAGER_H_
#define ORBIT_GL_TRACK_MANAGER_H_

#include <absl/base/thread_annotations.h>
#include <absl/container/flat_hash_map.h>
#include <absl/synchronization/mutex.h>
#include <stdint.h>
#include <stdlib.h>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <string_view>
#include <thread>
#include <utility>
#include <vector>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/TimerTrackDataIdManager.h"
#include "ClientProtos/capture_data.pb.h"
#include "OrbitGl/AsyncTrack.h"
#include "OrbitGl/CGroupAndProcessMemoryTrack.h"
#include "OrbitGl/FrameTrack.h"
#include "OrbitGl/GpuTrack.h"
#include "OrbitGl/PageFaultsTrack.h"
#include "OrbitGl/SchedulerTrack.h"
#include "OrbitGl/SystemMemoryTrack.h"
#include "OrbitGl/ThreadTrack.h"
#include "OrbitGl/TimeGraphLayout.h"
#include "OrbitGl/TimelineInfoInterface.h"
#include "OrbitGl/Timer.h"
#include "OrbitGl/Track.h"
#include "OrbitGl/VariableTrack.h"
#include "OrbitGl/Viewport.h"

class OrbitApp;

namespace orbit_gl {

class TrackContainer;

// TrackManager is in charge of the active Tracks in Timegraph (their creation, searching, erasing
// and sorting).
class TrackManager {
 public:
  explicit TrackManager(TrackContainer* track_container, TimelineInfoInterface* timeline_info,
                        Viewport* viewport, TimeGraphLayout* layout, OrbitApp* app,
                        const orbit_client_data::ModuleManager* module_manager,
                        orbit_client_data::CaptureData* capture_data,
                        std::thread::id main_thread_id = std::this_thread::get_id());

  [[nodiscard]] std::vector<Track*> GetAllTracks() const;
  [[nodiscard]] const std::vector<Track*>& GetVisibleTracks() const { return visible_tracks_; }
  [[nodiscard]] std::vector<FrameTrack*> GetFrameTracks() const;

  void RequestTrackSorting() { sorting_invalidated_ = true; };
  void SetFilter(std::string_view filter);

  void UpdateTrackListForRendering();

  [[nodiscard]] std::pair<uint64_t, uint64_t> GetTracksMinMaxTimestamps() const;

  [[nodiscard]] static bool IteratableType(orbit_client_protos::TimerInfo_Type type);
  [[nodiscard]] static bool FunctionIteratableType(orbit_client_protos::TimerInfo_Type type);

  Track* GetOrCreateTrackFromTimerInfo(const orbit_client_protos::TimerInfo& timer_info);
  SchedulerTrack* GetOrCreateSchedulerTrack();
  ThreadTrack* GetOrCreateThreadTrack(uint32_t tid);
  [[nodiscard]] std::optional<ThreadTrack*> GetThreadTrack(uint32_t tid) const;
  GpuTrack* GetOrCreateGpuTrack(uint64_t timeline_hash);
  VariableTrack* GetOrCreateVariableTrack(std::string_view name);
  AsyncTrack* GetOrCreateAsyncTrack(std::string_view name);
  FrameTrack* GetOrCreateFrameTrack(uint64_t function_id);
  [[nodiscard]] SystemMemoryTrack* GetSystemMemoryTrack() const;
  [[nodiscard]] SystemMemoryTrack* CreateAndGetSystemMemoryTrack();
  [[nodiscard]] CGroupAndProcessMemoryTrack* GetCGroupAndProcessMemoryTrack() const;
  [[nodiscard]] CGroupAndProcessMemoryTrack* CreateAndGetCGroupAndProcessMemoryTrack(
      std::string_view cgroup_name);
  PageFaultsTrack* GetPageFaultsTrack() const;
  PageFaultsTrack* CreateAndGetPageFaultsTrack(std::string_view cgroup_name,
                                               uint64_t memory_sampling_period_ms);

  void SetIsDataFromSavedCapture(bool value) { data_from_saved_capture_ = value; }

  void RemoveFrameTrack(uint64_t function_id);

  void SetTrackTypeVisibility(Track::Type type, bool value);
  [[nodiscard]] bool GetTrackTypeVisibility(Track::Type type) const;

  const absl::flat_hash_map<Track::Type, bool> GetAllTrackTypesVisibility() const;
  void RestoreAllTrackTypesVisibility(const absl::flat_hash_map<Track::Type, bool>& values);

 private:
  [[nodiscard]] int FindMovingTrackIndex();
  void UpdateMovingTrackPositionInVisibleTracks();
  void SortTracks();
  [[nodiscard]] std::vector<ThreadTrack*> GetSortedThreadTracks()
      ABSL_SHARED_LOCKS_REQUIRED(mutex_);
  [[nodiscard]] std::vector<Track*> GetAllTracksInternal() const ABSL_SHARED_LOCKS_REQUIRED(mutex_);
  ThreadTrack* GetOrCreateThreadTrackInternal(uint32_t tid) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  // Filter tracks that are already sorted in sorted_tracks_.
  void UpdateVisibleTrackList();
  void DeletePendingTracks();
  void InsertPendingFrameTracks();

  void AddTrack(const std::shared_ptr<Track>& track) ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);
  void AddFrameTrack(const std::shared_ptr<FrameTrack>& frame_track)
      ABSL_EXCLUSIVE_LOCKS_REQUIRED(mutex_);

  const std::thread::id main_thread_id_;

  // This mutex is needed because two different threads will access the following containers. The
  // capture thread will insert tracks and the main thread will read the tracks from the containers
  // to generate the sorted list of visible tracks.
  mutable absl::Mutex mutex_;

  std::vector<std::shared_ptr<Track>> all_tracks_ ABSL_GUARDED_BY(mutex_);
  absl::flat_hash_map<uint32_t, std::shared_ptr<ThreadTrack>> thread_tracks_
      ABSL_GUARDED_BY(mutex_);
  std::map<std::string, std::shared_ptr<AsyncTrack>, std::less<>> async_tracks_
      ABSL_GUARDED_BY(mutex_);
  std::map<std::string, std::shared_ptr<VariableTrack>, std::less<>> variable_tracks_
      ABSL_GUARDED_BY(mutex_);
  // Mapping from timeline to GPU tracks. Timeline name is used for stable ordering. In particular
  // we want the marker tracks next to their queue track. E.g. "gfx" and "gfx_markers" should appear
  // next to each other.
  std::map<std::string, std::shared_ptr<GpuTrack>> gpu_tracks_ ABSL_GUARDED_BY(mutex_);
  // Mapping from function id to frame tracks.
  std::map<uint64_t, std::shared_ptr<FrameTrack>> frame_tracks_ ABSL_GUARDED_BY(mutex_);
  std::shared_ptr<SchedulerTrack> scheduler_track_ ABSL_GUARDED_BY(mutex_);
  std::shared_ptr<SystemMemoryTrack> system_memory_track_ ABSL_GUARDED_BY(mutex_);
  std::shared_ptr<CGroupAndProcessMemoryTrack> cgroup_and_process_memory_track_
      ABSL_GUARDED_BY(mutex_);
  std::shared_ptr<PageFaultsTrack> page_faults_track_ ABSL_GUARDED_BY(mutex_);

  // This intermediatly stores inserted frame tracks. We want to make sure that frame tracks are
  // inserted into the visible tracks during UpdateLayout() but as they could be added later by a
  // user, we don't want to invalidate the sorting of the remaining tracks.
  std::vector<std::shared_ptr<FrameTrack>> pending_frame_tracks_ ABSL_GUARDED_BY(mutex_);

  // This intermediatly stores tracks that have been deleted from one of the track vectors above
  // so they can safely be removed from the list of sorted and visible tracks.
  // This makes sure tracks are always removed during UpdateLayout(), so no elements retain
  // stale pointers returned in GetAllChildren() or GetNonHiddenChildren(). It should only be
  // accessed by the main_thread.
  std::vector<std::shared_ptr<Track>> deleted_tracks_;

  Viewport* viewport_;
  TimeGraphLayout* layout_;

  // While capturing, tracks are sorted internally into sorted_tracks_ using the function
  // SortTracks(). After the capture is finished, addition and deletion of tracks won't trigger a
  // reordering of the remaining tracks. visible_tracks_ is a subsequence of sorted_tracks_ filtered
  // by the word in filter_ and the visibility of each track individually and its type_visibility.
  // Both arrays should only be accessed by the main_thread_.
  std::vector<Track*> sorted_tracks_;
  Timer last_thread_reorder_;
  bool sorting_invalidated_ = false;
  std::vector<Track*> visible_tracks_;
  std::string filter_;
  bool visible_track_list_needs_update_ = false;
  absl::flat_hash_map<Track::Type, bool> track_type_visibility_;

  const orbit_client_data::ModuleManager* module_manager_;
  orbit_client_data::CaptureData* capture_data_ = nullptr;

  OrbitApp* app_ = nullptr;
  TrackContainer* track_container_ = nullptr;
  TimelineInfoInterface* timeline_info_ = nullptr;

  bool data_from_saved_capture_ = false;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_TRACK_MANAGER_H_
