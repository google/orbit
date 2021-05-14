// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_TRACK_MANAGER_H_
#define ORBIT_GL_TRACK_MANAGER_H_

#include <absl/container/flat_hash_map.h>
#include <stdint.h>
#include <stdlib.h>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "AsyncTrack.h"
#include "FrameTrack.h"
#include "GpuTrack.h"
#include "GraphTrack.h"
#include "MemoryTrack.h"
#include "PickingManager.h"
#include "SchedulerTrack.h"
#include "StringManager.h"
#include "ThreadTrack.h"
#include "Timer.h"
#include "Track.h"
#include "Viewport.h"
#include "capture_data.pb.h"

class OrbitApp;
class Timegraph;

// TrackManager is in charge of the active Tracks in Timegraph (their creation, searching, erasing
// and sorting).
class TrackManager {
 public:
  explicit TrackManager(TimeGraph* time_graph, orbit_gl::Viewport* viewport,
                        TimeGraphLayout* layout, OrbitApp* app,
                        const orbit_client_model::CaptureData* capture_data);

  [[nodiscard]] std::vector<Track*> GetAllTracks() const;
  [[nodiscard]] std::vector<Track*> GetVisibleTracks() const { return visible_tracks_; }
  [[nodiscard]] std::vector<ThreadTrack*> GetThreadTracks() const;
  [[nodiscard]] std::vector<FrameTrack*> GetFrameTracks() const;

  [[nodiscard]] ThreadTrack* GetTracepointsSystemWideTrack() const {
    return tracepoints_system_wide_track_;
  }

  void RequestTrackSorting() { sorting_invalidated_ = true; };
  void SetFilter(const std::string& filter);

  void UpdateTracks(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                    PickingMode picking_mode);
  [[nodiscard]] float GetTracksTotalHeight() const { return tracks_total_height_; }

  [[nodiscard]] uint32_t GetNumTimers() const;
  [[nodiscard]] std::pair<uint64_t, uint64_t> GetTracksMinMaxTimestamps() const;

  SchedulerTrack* GetOrCreateSchedulerTrack();
  ThreadTrack* GetOrCreateThreadTrack(int32_t tid);
  GpuTrack* GetOrCreateGpuTrack(uint64_t timeline_hash);
  GraphTrack* GetOrCreateGraphTrack(const std::string& name);
  orbit_gl::MemoryTrack* GetOrCreateMemoryTrack(const std::string& name);
  AsyncTrack* GetOrCreateAsyncTrack(const std::string& name);
  FrameTrack* GetOrCreateFrameTrack(const orbit_grpc_protos::InstrumentedFunction& function);

  [[nodiscard]] bool GetIsDataFromSavedCapture() const { return data_from_saved_capture_; }
  void SetIsDataFromSavedCapture(bool value) { data_from_saved_capture_ = value; }

  void RemoveFrameTrack(uint64_t function_address);

 private:
  void UpdateTracksOrder();
  [[nodiscard]] int FindMovingTrackIndex();
  void UpdateMovingTrackSorting();
  void SortTracks();
  [[nodiscard]] std::vector<ThreadTrack*> GetSortedThreadTracks();
  void UpdateVisibleTrackList();

  void AddTrack(const std::shared_ptr<Track>& track);
  void AddFrameTrack(const std::shared_ptr<FrameTrack>& frame_track);

  // TODO(b/174655559): Use absl's mutex here.
  mutable std::recursive_mutex mutex_;

  std::vector<std::shared_ptr<Track>> all_tracks_;
  absl::flat_hash_map<int32_t, std::shared_ptr<ThreadTrack>> thread_tracks_;
  std::map<std::string, std::shared_ptr<AsyncTrack>> async_tracks_;
  std::map<std::string, std::shared_ptr<GraphTrack>> graph_tracks_;
  std::map<std::string, std::shared_ptr<orbit_gl::MemoryTrack>> memory_tracks_;
  // Mapping from timeline to GPU tracks. Timeline name is used for stable ordering. In particular
  // we want the marker tracks next to their queue track. E.g. "gfx" and "gfx_markers" should appear
  // next to each other.
  std::map<std::string, std::shared_ptr<GpuTrack>> gpu_tracks_;
  // Mapping from function address to frame tracks.
  // TODO(b/175865913): Use Function info instead of their address as key to FrameTracks
  std::map<uint64_t, std::shared_ptr<FrameTrack>> frame_tracks_;
  std::shared_ptr<SchedulerTrack> scheduler_track_;
  ThreadTrack* tracepoints_system_wide_track_;

  TimeGraph* time_graph_;
  orbit_gl::Viewport* viewport_;
  TimeGraphLayout* layout_;

  std::vector<Track*> sorted_tracks_;
  bool sorting_invalidated_ = false;
  bool visible_track_list_needs_update_ = false;
  Timer last_thread_reorder_;

  std::string filter_;
  std::vector<Track*> visible_tracks_;

  float tracks_total_height_ = 0.0f;
  const orbit_client_model::CaptureData* capture_data_ = nullptr;

  OrbitApp* app_ = nullptr;

  bool data_from_saved_capture_ = false;
};

#endif  // ORBIT_GL_TRACK_MANAGER_H_
