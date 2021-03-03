// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TrackManager.h"

#include <GteVector.h>
#include <absl/container/flat_hash_map.h>
#include <absl/strings/ascii.h>
#include <absl/strings/match.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <stdlib.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "App.h"
#include "CoreMath.h"
#include "CoreUtils.h"
#include "GlCanvas.h"
#include "OrbitBase/ThreadConstants.h"
#include "OrbitClientData/CallstackData.h"
#include "OrbitClientModel/CaptureData.h"
#include "TimeGraph.h"
#include "TimeGraphLayout.h"

using orbit_client_protos::FunctionInfo;

TrackManager::TrackManager(TimeGraph* time_graph, TimeGraphLayout* layout, OrbitApp* app,
                           const CaptureData* capture_data)
    : time_graph_(time_graph), layout_(layout), capture_data_{capture_data}, app_{app} {
  GetOrCreateSchedulerTrack();
  tracepoints_system_wide_track_ = GetOrCreateThreadTrack(orbit_base::kAllThreadsOfAllProcessesTid);
}

std::vector<Track*> TrackManager::GetAllTracks() const {
  std::vector<Track*> tracks;
  for (const auto& track : all_tracks_) {
    tracks.push_back(track.get());
  }
  return tracks;
}

std::vector<ThreadTrack*> TrackManager::GetThreadTracks() const {
  std::vector<ThreadTrack*> tracks;
  for (auto& [unused_key, track] : thread_tracks_) {
    tracks.push_back(track.get());
  }
  return tracks;
}

std::vector<FrameTrack*> TrackManager::GetFrameTracks() const {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::vector<FrameTrack*> tracks;
  for (auto& [unused_id, track] : frame_tracks_) {
    tracks.push_back(track.get());
  }
  return tracks;
}

void TrackManager::SortTracks() {
  if (!app_->IsCapturing() && !sorted_tracks_.empty() && !sorting_invalidated_) return;

  // Reorder threads if sorting isn't valid or once per second when capturing
  if (sorting_invalidated_ || last_thread_reorder_.ElapsedMillis() > 1000.0) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // Gather all tracks regardless of the process in sorted order
    std::vector<Track*> all_processes_sorted_tracks;

    // Gpu tracks.
    for (const auto& timeline_and_track : gpu_tracks_) {
      all_processes_sorted_tracks.push_back(timeline_and_track.second.get());
    }

    // Frame tracks.
    for (const auto& name_and_track : frame_tracks_) {
      all_processes_sorted_tracks.push_back(name_and_track.second.get());
    }

    // Graph tracks.
    for (const auto& graph_track : graph_tracks_) {
      all_processes_sorted_tracks.push_back(graph_track.second.get());
    }

    // Async tracks.
    for (const auto& async_track : async_tracks_) {
      all_processes_sorted_tracks.push_back(async_track.second.get());
    }

    // Tracepoint tracks.
    if (!tracepoints_system_wide_track_->IsEmpty()) {
      all_processes_sorted_tracks.push_back(tracepoints_system_wide_track_);
    }

    // Process track.
    if (app_->HasCaptureData()) {
      ThreadTrack* process_track = GetOrCreateThreadTrack(orbit_base::kAllProcessThreadsTid);
      if (!process_track->IsEmpty()) {
        all_processes_sorted_tracks.push_back(process_track);
      }
    }

    // Thread tracks.
    std::vector<ThreadTrack*> sorted_thread_tracks = GetSortedThreadTracks();
    for (ThreadTrack* thread_track : sorted_thread_tracks) {
      if (!thread_track->IsEmpty()) {
        all_processes_sorted_tracks.push_back(thread_track);
      }
    }

    // Separate "capture_pid" tracks from tracks that originate from other processes.
    int32_t capture_pid = capture_data_ ? capture_data_->process_id() : 0;
    std::vector<Track*> capture_pid_tracks;
    std::vector<Track*> external_pid_tracks;
    for (auto& track : all_processes_sorted_tracks) {
      int32_t pid = track->GetProcessId();
      if (pid != -1 && pid != capture_pid) {
        external_pid_tracks.push_back(track);
      } else {
        capture_pid_tracks.push_back(track);
      }
    }

    // Clear before repopulating.
    sorted_tracks_.clear();

    // Scheduler track.
    if (!scheduler_track_->IsEmpty()) {
      sorted_tracks_.push_back(scheduler_track_.get());
    }

    // For now, "external_pid_tracks" should only contain
    // introspection tracks. Display them on top.
    Append(sorted_tracks_, external_pid_tracks);
    Append(sorted_tracks_, capture_pid_tracks);

    last_thread_reorder_.Restart();

    UpdateFilteredTrackList();
  }
  sorting_invalidated_ = false;
}

void TrackManager::SetFilter(const std::string& filter) {
  filter_ = absl::AsciiStrToLower(filter);
  UpdateFilteredTrackList();
}

void TrackManager::UpdateFilteredTrackList() {
  if (filter_.empty()) {
    visible_tracks_ = sorted_tracks_;
    return;
  }

  visible_tracks_.clear();
  std::vector<std::string> filters = absl::StrSplit(filter_, ' ', absl::SkipWhitespace());
  for (const auto& track : sorted_tracks_) {
    std::string lower_case_label = absl::AsciiStrToLower(track->GetLabel());
    for (auto& filter : filters) {
      if (absl::StrContains(lower_case_label, filter) ||
          track->GetType() == Track::kSchedulerTrack) {
        visible_tracks_.push_back(track);
        break;
      }
    }
  }
}

std::vector<ThreadTrack*> TrackManager::GetSortedThreadTracks() {
  std::vector<ThreadTrack*> sorted_tracks;
  absl::flat_hash_map<ThreadTrack*, uint32_t> num_events_by_track;
  const CallstackData* callstack_data = capture_data_ ? capture_data_->GetCallstackData() : nullptr;

  for (auto& [tid, track] : thread_tracks_) {
    if (tid == orbit_base::kAllProcessThreadsTid) {
      continue;  // "kAllProcessThreadsTid" is handled separately.
    }
    sorted_tracks.push_back(track.get());
    uint32_t num_events = callstack_data ? callstack_data->GetCallstackEventsOfTidCount(tid) : 0;
    num_events_by_track[track.get()] = num_events;
  }

  // Tracks with instrumented timers appear first, ordered by descending order of timers.
  // The remaining tracks appear after, ordered by descending order of callstack events.
  std::sort(sorted_tracks.begin(), sorted_tracks.end(),
            [&num_events_by_track](ThreadTrack* a, ThreadTrack* b) {
              return std::make_tuple(a->GetNumTimers(), num_events_by_track[a]) >
                     std::make_tuple(b->GetNumTimers(), num_events_by_track[b]);
            });

  return sorted_tracks;
}

void TrackManager::UpdateMovingTrackSorting() {
  // This updates the position of the currently moving track in both the sorted_tracks_
  // and the visible_tracks_ array. The moving track is inserted after the first track
  // with a value of top + height smaller than the current mouse position.
  // Only drawn (i.e. not filtered out) tracks are taken into account to determine the
  // insertion position, but both arrays are updated accordingly.
  //
  // Note: We do an O(n) search for the correct position in the sorted_tracks_ array which
  // could be optimized, but this is not worth the effort for the limited number of tracks.

  int moving_track_previous_position = FindMovingTrackIndex();

  if (moving_track_previous_position != -1) {
    Track* moving_track = visible_tracks_[moving_track_previous_position];
    visible_tracks_.erase(visible_tracks_.begin() + moving_track_previous_position);

    int moving_track_current_position = -1;
    for (auto track_it = visible_tracks_.begin(); track_it != visible_tracks_.end(); ++track_it) {
      if (moving_track->GetPos()[1] >= (*track_it)->GetPos()[1]) {
        auto inserted_it = visible_tracks_.insert(track_it, moving_track);
        moving_track_current_position = inserted_it - visible_tracks_.begin();
        break;
      }
    }

    if (moving_track_current_position == -1) {
      visible_tracks_.push_back(moving_track);
      moving_track_current_position = static_cast<int>(visible_tracks_.size()) - 1;
    }

    // Now we have to change the position of the moving_track in the non-filtered array
    if (moving_track_current_position == moving_track_previous_position) {
      return;
    }
    sorted_tracks_.erase(std::find(sorted_tracks_.begin(), sorted_tracks_.end(), moving_track));
    if (moving_track_current_position > moving_track_previous_position) {
      // In this case we will insert the moving_track right after the one who is before in the
      // filtered array
      Track* previous_filtered_track = visible_tracks_[moving_track_current_position - 1];
      sorted_tracks_.insert(
          ++std::find(sorted_tracks_.begin(), sorted_tracks_.end(), previous_filtered_track),
          moving_track);
    } else {
      // In this case we will insert the moving_track right before the one who is after in the
      // filtered array
      Track* next_filtered_track = visible_tracks_[moving_track_current_position + 1];
      sorted_tracks_.insert(
          std::find(sorted_tracks_.begin(), sorted_tracks_.end(), next_filtered_track),
          moving_track);
    }
  }
}

int TrackManager::FindMovingTrackIndex() {
  // Returns the position of the moving track, or -1 if there is none.
  for (auto track_it = visible_tracks_.begin(); track_it != visible_tracks_.end(); ++track_it) {
    if ((*track_it)->IsMoving()) {
      return track_it - visible_tracks_.begin();
    }
  }
  return -1;
}

void TrackManager::UpdateTracks(Batcher* batcher, uint64_t min_tick, uint64_t max_tick,
                                PickingMode picking_mode) {
  // Make sure track tab fits in the viewport.
  float current_y = -layout_->GetSchedulerTrackOffset() - layout_->GetTrackTabHeight();
  float pinned_tracks_height = 0.f;

  // Draw pinned tracks
  for (auto& track : visible_tracks_) {
    if (!track->IsPinned()) {
      continue;
    }

    const float z_offset = GlCanvas::kZOffsetPinnedTrack;
    if (!track->IsMoving()) {
      track->SetPos(track->GetPos()[0], current_y + time_graph_->GetCanvas()->GetWorldTopLeftY() -
                                            layout_->GetTopMargin() -
                                            layout_->GetSchedulerTrackOffset());
    }
    track->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
    const float height = (track->GetHeight() + layout_->GetSpaceBetweenTracks());
    current_y -= height;
    pinned_tracks_height += height;
  }

  // Draw unpinned tracks
  for (auto& track : visible_tracks_) {
    if (track->IsPinned()) {
      continue;
    }

    const float z_offset = track->IsMoving() ? GlCanvas::kZOffsetMovingTack : 0.f;
    if (!track->IsMoving()) {
      track->SetPos(track->GetPos()[0], current_y);
    }
    track->UpdatePrimitives(batcher, min_tick, max_tick, picking_mode, z_offset);
    current_y -= (track->GetHeight() + layout_->GetSpaceBetweenTracks());
  }

  // Tracks are drawn from 0 (top) to negative y-coordinates.
  tracks_total_height_ = std::abs(current_y);
}

void TrackManager::AddTrack(const std::shared_ptr<Track>& track) {
  all_tracks_.push_back(track);
  sorting_invalidated_ = true;
}

void TrackManager::RemoveFrameTrack(uint64_t function_id) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  frame_tracks_.erase(function_id);
  sorting_invalidated_ = true;
  // We need to do SortTracks again to have visible_tracks_ updated
  SortTracks();
}

SchedulerTrack* TrackManager::GetOrCreateSchedulerTrack() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<SchedulerTrack> track = scheduler_track_;
  if (track == nullptr) {
    track = std::make_shared<SchedulerTrack>(time_graph_, layout_, app_, capture_data_);
    AddTrack(track);
    scheduler_track_ = track;
  }
  return track.get();
}

ThreadTrack* TrackManager::GetOrCreateThreadTrack(int32_t tid) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<ThreadTrack> track = thread_tracks_[tid];
  if (track == nullptr) {
    track = std::make_shared<ThreadTrack>(time_graph_, layout_, tid, app_, capture_data_);
    AddTrack(track);
    thread_tracks_[tid] = track;
  }
  return track.get();
}

GpuTrack* TrackManager::GetOrCreateGpuTrack(uint64_t timeline_hash) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::string timeline =
      app_->GetStringManager()->Get(timeline_hash).value_or(std::to_string(timeline_hash));
  std::shared_ptr<GpuTrack> track = gpu_tracks_[timeline];
  if (track == nullptr) {
    track = std::make_shared<GpuTrack>(time_graph_, layout_, timeline_hash, app_, capture_data_);
    AddTrack(track);
    gpu_tracks_[timeline] = track;
  }
  return track.get();
}

GraphTrack* TrackManager::GetOrCreateGraphTrack(const std::string& name) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<GraphTrack> track = graph_tracks_[name];
  if (track == nullptr) {
    track = std::make_shared<GraphTrack>(time_graph_, layout_, name, capture_data_);
    AddTrack(track);
    graph_tracks_[name] = track;
  }
  return track.get();
}

AsyncTrack* TrackManager::GetOrCreateAsyncTrack(const std::string& name) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<AsyncTrack> track = async_tracks_[name];
  if (track == nullptr) {
    track = std::make_shared<AsyncTrack>(time_graph_, layout_, name, app_, capture_data_);
    AddTrack(track);
    async_tracks_[name] = track;
  }

  return track.get();
}

FrameTrack* TrackManager::GetOrCreateFrameTrack(uint64_t function_id,
                                                const FunctionInfo& function) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  auto track_it = frame_tracks_.find(function_id);
  if (track_it != frame_tracks_.end()) {
    return track_it->second.get();
  }

  auto track = std::make_shared<FrameTrack>(time_graph_, layout_, function_id, function, app_,
                                            capture_data_);

  // Normally we would call AddTrack(track) here, but frame tracks are removable by users
  // and therefore cannot be simply thrown into the flat vector of tracks.
  sorting_invalidated_ = true;
  frame_tracks_[function_id] = track;

  return track.get();
}
