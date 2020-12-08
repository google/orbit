// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <unordered_map>
#include <utility>

#include "AsyncTrack.h"
#include "FrameTrack.h"
#include "GpuTrack.h"
#include "GraphTrack.h"
#include "SchedulerTrack.h"
#include "ThreadTrack.h"

class Timegraph;

// TrackManager is in charge of the active Tracks in Timegraph (their creation, searching, erasing
// and sorting).
class TrackManager {
 public:
  explicit TrackManager(TimeGraph* time_graph);

  void Clear();

  [[nodiscard]] std::vector<std::shared_ptr<Track>> GetTracks() const;
  [[nodiscard]] std::vector<std::shared_ptr<Track>> GetFilteredTracks() const {
    return sorted_filtered_tracks_;
  }
  [[nodiscard]] std::vector<std::shared_ptr<ThreadTrack>> GetThreadTracks() const;
  [[nodiscard]] std::vector<std::shared_ptr<FrameTrack>> GetFrameTracks() const;
  [[nodiscard]] std::vector<std::shared_ptr<AsyncTrack>> GetAsyncTracks() const;
  [[nodiscard]] std::vector<std::shared_ptr<GraphTrack>> GetGraphTracks() const;
  [[nodiscard]] std::vector<std::shared_ptr<GpuTrack>> GetGpuTracks() const;

  [[nodiscard]] std::shared_ptr<SchedulerTrack> GetSchedulerTrack() const {
    return scheduler_track_;
  };
  [[nodiscard]] std::shared_ptr<ThreadTrack> GetTracepointsSystemWideTrack() const {
    return tracepoints_system_wide_track_;
  }

  void SetStringManager(std::shared_ptr<StringManager> str_manager);
  [[nodiscard]] std::shared_ptr<StringManager> GetStringManager() { return string_manager_; }

  void SortTracks();
  void SetFilter(const std::string& filter);

  void UpdateTracks(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode);
  [[nodiscard]] float GetTracksHeight() const { return tracks_height_; }

  std::shared_ptr<SchedulerTrack> GetOrCreateSchedulerTrack();
  std::shared_ptr<ThreadTrack> GetOrCreateThreadTrack(int32_t tid);
  std::shared_ptr<GpuTrack> GetOrCreateGpuTrack(uint64_t timeline_hash);
  GraphTrack* GetOrCreateGraphTrack(const std::string& name);
  AsyncTrack* GetOrCreateAsyncTrack(const std::string& name);
  std::shared_ptr<FrameTrack> GetOrCreateFrameTrack(
      const orbit_client_protos::FunctionInfo& function);

  void AddTrack(std::shared_ptr<Track> track);
  void RemoveFrameTrack(uint64_t function_address);

  void UpdateMovingTrackSorting();

 private:
  void UpdateFilteredTrackList();
  [[nodiscard]] int FindMovingTrackIndex();
  [[nodiscard]] std::vector<int32_t> GetSortedThreadIds();

  mutable std::recursive_mutex mutex_;

  std::vector<std::shared_ptr<Track>> tracks_;
  std::unordered_map<int32_t, std::shared_ptr<ThreadTrack>> thread_tracks_;
  std::map<std::string, std::shared_ptr<AsyncTrack>> async_tracks_;
  std::map<std::string, std::shared_ptr<GraphTrack>> graph_tracks_;
  // Mapping from timeline hash to GPU tracks.
  std::unordered_map<uint64_t, std::shared_ptr<GpuTrack>> gpu_tracks_;
  // Mapping from function address to frame tracks.
  std::unordered_map<uint64_t, std::shared_ptr<FrameTrack>> frame_tracks_;
  std::shared_ptr<SchedulerTrack> scheduler_track_;
  std::shared_ptr<ThreadTrack> tracepoints_system_wide_track_;

  TimeGraph* time_graph_;

  std::vector<std::shared_ptr<Track>> sorted_tracks_;
  bool sorting_invalidated_;
  Timer last_thread_reorder_;
  std::map<int32_t, uint32_t> thread_count_map_;
  std::map<int32_t, uint32_t> event_count_;

  std::string filter_;
  std::vector<std::shared_ptr<Track>> sorted_filtered_tracks_;
  float tracks_height_;
  std::shared_ptr<StringManager> string_manager_;
};
