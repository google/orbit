// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TrackManager.h"

#include "App.h"
#include "CoreUtils.h"
#include "OrbitBase/ThreadConstants.h"

using orbit_client_protos::FunctionInfo;

TrackManager::TrackManager(TimeGraph* time_graph, OrbitApp* app)
    : time_graph_(time_graph), app_{app} {
  scheduler_track_ = GetOrCreateSchedulerTrack();

  tracepoints_system_wide_track_ = GetOrCreateThreadTrack(orbit_base::kAllThreadsOfAllProcessesTid);
}

void TrackManager::Clear() {
  tracks_.clear();
  scheduler_track_ = nullptr;
  thread_tracks_.clear();
  gpu_tracks_.clear();
  graph_tracks_.clear();
  async_tracks_.clear();
  frame_tracks_.clear();

  sorted_tracks_.clear();
  sorted_filtered_tracks_.clear();

  scheduler_track_ = GetOrCreateSchedulerTrack();
  tracepoints_system_wide_track_ = GetOrCreateThreadTrack(orbit_base::kAllThreadsOfAllProcessesTid);
}

void TrackManager::SetStringManager(StringManager* str_manager) { string_manager_ = str_manager; }

std::vector<std::shared_ptr<Track>> TrackManager::GetTracks() const { return tracks_; }

std::vector<std::shared_ptr<ThreadTrack>> TrackManager::GetThreadTracks() const {
  std::vector<std::shared_ptr<ThreadTrack>> tracks;
  for (auto& [unused_key, thread_track] : thread_tracks_) {
    tracks.emplace_back(thread_track);
  }
  return tracks;
}

std::vector<std::shared_ptr<FrameTrack>> TrackManager::GetFrameTracks() const {
  std::vector<std::shared_ptr<FrameTrack>> tracks;
  for (auto& [unused_key, track] : frame_tracks_) {
    tracks.emplace_back(track);
  }
  return tracks;
}

std::vector<std::shared_ptr<AsyncTrack>> TrackManager::GetAsyncTracks() const {
  std::vector<std::shared_ptr<AsyncTrack>> tracks;
  for (auto& [unused_key, track] : async_tracks_) {
    tracks.emplace_back(track);
  }
  return tracks;
}

std::vector<std::shared_ptr<GraphTrack>> TrackManager::GetGraphTracks() const {
  std::vector<std::shared_ptr<GraphTrack>> tracks;
  for (auto& [unused_key, track] : graph_tracks_) {
    tracks.emplace_back(track);
  }
  return tracks;
}

std::vector<std::shared_ptr<GpuTrack>> TrackManager::GetGpuTracks() const {
  std::vector<std::shared_ptr<GpuTrack>> tracks;
  for (auto& [unused_key, track] : gpu_tracks_) {
    tracks.emplace_back(track);
  }
  return tracks;
}

void TrackManager::SortTracks() {
  if (!app_->IsCapturing() && !sorted_tracks_.empty() && !sorting_invalidated_) return;

  std::shared_ptr<ThreadTrack> process_track = nullptr;

  const CaptureData* capture_data = time_graph_->GetCaptureData();
  if (capture_data != nullptr) {
    // Get or create thread track from events' thread id.
    event_count_.clear();
    event_count_[orbit_base::kAllProcessThreadsTid] =
        capture_data->GetCallstackData()->GetCallstackEventsCount();

    // The process track is a special ThreadTrack of id "kAllThreadsFakeTid".
    process_track = GetOrCreateThreadTrack(orbit_base::kAllProcessThreadsTid);
    for (const auto& tid_and_count :
         capture_data->GetCallstackData()->GetCallstackEventsCountsPerTid()) {
      const int32_t thread_id = tid_and_count.first;
      const uint32_t count = tid_and_count.second;
      event_count_[thread_id] = count;
      GetOrCreateThreadTrack(thread_id);
    }
  }

  // Reorder threads once every second when capturing
  if (!app_->IsCapturing() || last_thread_reorder_.ElapsedMillis() > 1000.0) {
    std::vector<int32_t> sorted_thread_ids = GetSortedThreadIds();

    std::lock_guard<std::recursive_mutex> lock(mutex_);
    // Gather all tracks regardless of the process in sorted order
    std::vector<std::shared_ptr<Track>> all_processes_sorted_tracks;

    // Gpu tracks.
    for (const auto& timeline_and_track : gpu_tracks_) {
      all_processes_sorted_tracks.emplace_back(timeline_and_track.second);
    }

    // Frame tracks.
    for (const auto& name_and_track : frame_tracks_) {
      all_processes_sorted_tracks.emplace_back(name_and_track.second);
    }

    // Graph tracks.
    for (const auto& graph_track : graph_tracks_) {
      all_processes_sorted_tracks.emplace_back(graph_track.second);
    }

    // Async tracks.
    for (const auto& async_track : async_tracks_) {
      all_processes_sorted_tracks.emplace_back(async_track.second);
    }

    // Tracepoint tracks.
    if (!tracepoints_system_wide_track_->IsEmpty()) {
      all_processes_sorted_tracks.emplace_back(tracepoints_system_wide_track_);
    }

    // Process track.
    if (process_track && !process_track->IsEmpty()) {
      all_processes_sorted_tracks.emplace_back(process_track);
    }

    // Thread tracks.
    for (auto thread_id : sorted_thread_ids) {
      std::shared_ptr<ThreadTrack> track = GetOrCreateThreadTrack(thread_id);
      if (!track->IsEmpty()) {
        all_processes_sorted_tracks.emplace_back(track);
      }
    }

    // Separate "capture_pid" tracks from tracks that originate from other processes.
    int32_t capture_pid = capture_data ? capture_data->process_id() : 0;
    std::vector<std::shared_ptr<Track>> capture_pid_tracks;
    std::vector<std::shared_ptr<Track>> external_pid_tracks;
    for (auto& track : all_processes_sorted_tracks) {
      int32_t pid = track->GetProcessId();
      if (pid != -1 && pid != capture_pid) {
        external_pid_tracks.emplace_back(track);
      } else {
        capture_pid_tracks.emplace_back(track);
      }
    }

    // Clear before repopulating.
    sorted_tracks_.clear();

    // Scheduler track.
    if (!scheduler_track_->IsEmpty()) {
      sorted_tracks_.emplace_back(scheduler_track_);
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
    sorted_filtered_tracks_ = sorted_tracks_;
    return;
  }

  sorted_filtered_tracks_.clear();
  std::vector<std::string> filters = absl::StrSplit(filter_, ' ', absl::SkipWhitespace());
  for (const auto& track : sorted_tracks_) {
    std::string lower_case_label = absl::AsciiStrToLower(track->GetLabel());
    for (auto& filter : filters) {
      if (absl::StrContains(lower_case_label, filter)) {
        sorted_filtered_tracks_.emplace_back(track);
        break;
      }
    }
  }
}

std::vector<int32_t>
TrackManager::GetSortedThreadIds() {  // Show threads with instrumented functions first
  std::vector<int32_t> sorted_thread_ids;
  std::vector<std::pair<int32_t, uint32_t>> sorted_threads =
      orbit_core::ReverseValueSort(thread_count_map_);

  for (auto& [tid, unused_value] : sorted_threads) {
    // Track "kAllThreadsFakeTid" holds all target process sampling info, it is handled
    // separately.
    if (tid != orbit_base::kAllProcessThreadsTid) {
      sorted_thread_ids.push_back(tid);
    }
  }

  // Then show threads sorted by number of events
  std::vector<std::pair<int32_t, uint32_t>> sorted_by_events =
      orbit_core::ReverseValueSort(event_count_);
  for (auto& [tid, unused_value] : sorted_by_events) {
    // Track "kAllThreadsFakeTid" holds all target process sampling info, it is handled
    // separately.
    if (tid == orbit_base::kAllProcessThreadsTid) continue;
    if (thread_count_map_.find(tid) == thread_count_map_.end()) {
      sorted_thread_ids.push_back(tid);
    }
  }

  return sorted_thread_ids;
}

void TrackManager::UpdateMovingTrackSorting() {
  // This updates the position of the currently moving track in both the sorted_tracks_
  // and the sorted_filtered_tracks_ array. The moving track is inserted after the first track
  // with a value of top + height smaller than the current mouse position.
  // Only drawn (i.e. not filtered out) tracks are taken into account to determine the
  // insertion position, but both arrays are updated accordingly.
  //
  // Note: We do an O(n) search for the correct position in the sorted_tracks_ array which
  // could be optimized, but this is not worth the effort for the limited number of tracks.

  int moving_track_previous_position = FindMovingTrackIndex();

  if (moving_track_previous_position != -1) {
    std::shared_ptr<Track> moving_track = sorted_filtered_tracks_[moving_track_previous_position];
    sorted_filtered_tracks_.erase(sorted_filtered_tracks_.begin() + moving_track_previous_position);

    int moving_track_current_position = -1;
    for (auto track_it = sorted_filtered_tracks_.begin(); track_it != sorted_filtered_tracks_.end();
         ++track_it) {
      if (moving_track->GetPos()[1] >= (*track_it)->GetPos()[1]) {
        sorted_filtered_tracks_.insert(track_it, moving_track);
        moving_track_current_position = track_it - sorted_filtered_tracks_.begin();
        break;
      }
    }

    if (moving_track_current_position == -1) {
      sorted_filtered_tracks_.push_back(moving_track);
      moving_track_current_position = static_cast<int>(sorted_filtered_tracks_.size()) - 1;
    }

    // Now we have to change the position of the moving_track in the non-filtered array
    if (moving_track_current_position == moving_track_previous_position) {
      return;
    }
    sorted_tracks_.erase(std::find(sorted_tracks_.begin(), sorted_tracks_.end(), moving_track));
    if (moving_track_current_position > moving_track_previous_position) {
      // In this case we will insert the moving_track right after the one who is before in the
      // filtered array
      auto previous_filtered_track = sorted_filtered_tracks_[moving_track_current_position - 1];
      sorted_tracks_.insert(
          ++std::find(sorted_tracks_.begin(), sorted_tracks_.end(), previous_filtered_track),
          moving_track);
    } else {
      // In this case we will insert the moving_track right before the one who is after in the
      // filtered array
      auto next_filtered_track = sorted_filtered_tracks_[moving_track_current_position + 1];
      sorted_tracks_.insert(
          std::find(sorted_tracks_.begin(), sorted_tracks_.end(), next_filtered_track),
          moving_track);
    }
  }
}

int TrackManager::FindMovingTrackIndex() {
  // Returns the position of the moving track, or -1 if there is none.
  for (auto track_it = sorted_filtered_tracks_.begin(); track_it != sorted_filtered_tracks_.end();
       ++track_it) {
    if ((*track_it)->IsMoving()) {
      return track_it - sorted_filtered_tracks_.begin();
    }
  }
  return -1;
}

void TrackManager::UpdateTracks(uint64_t min_tick, uint64_t max_tick, PickingMode picking_mode) {
  TimeGraphLayout layout = time_graph_->GetLayout();

  // Make sure track tab fits in the viewport.
  float current_y = -layout.GetSchedulerTrackOffset() - layout.GetTrackTabHeight();
  float pinned_tracks_height = 0.f;

  // Draw pinned tracks
  for (auto& track : sorted_filtered_tracks_) {
    if (!track->IsPinned()) {
      continue;
    }

    const float z_offset = GlCanvas::kZOffsetPinnedTrack;
    track->SetY(current_y + time_graph_->GetCanvas()->GetWorldTopLeftY() - layout.GetTopMargin() -
                layout.GetSchedulerTrackOffset());
    track->UpdatePrimitives(min_tick, max_tick, picking_mode, z_offset);
    const float height = (track->GetHeight() + layout.GetSpaceBetweenTracks());
    current_y -= height;
    pinned_tracks_height += height;
  }

  // Draw unpinned tracks
  for (auto& track : sorted_filtered_tracks_) {
    if (track->IsPinned()) {
      continue;
    }

    const float z_offset = track->IsMoving() ? GlCanvas::kZOffsetMovingTack : 0.f;
    track->SetY(current_y);
    track->UpdatePrimitives(min_tick, max_tick, picking_mode, z_offset);
    current_y -= (track->GetHeight() + layout.GetSpaceBetweenTracks());
  }

  // Tracks are drawn from 0 (top) to negative y-coordinates.
  tracks_total_height_ = std::abs(current_y);
}

void TrackManager::AddTrack(std::shared_ptr<Track> track) {
  tracks_.emplace_back(track);
  sorting_invalidated_ = true;
}

void TrackManager::RemoveFrameTrack(uint64_t function_address) {
  frame_tracks_.erase(function_address);
  sorting_invalidated_ = true;
}

std::shared_ptr<SchedulerTrack> TrackManager::GetOrCreateSchedulerTrack() {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<SchedulerTrack> track = scheduler_track_;
  if (track == nullptr) {
    track = std::make_shared<SchedulerTrack>(time_graph_, app_);
    AddTrack(track);
    scheduler_track_ = track;
    uint32_t num_cores = time_graph_->GetNumCores();
    time_graph_->GetLayout().SetNumCores(num_cores);
    scheduler_track_->SetLabel(absl::StrFormat("Scheduler (%u cores)", num_cores));
  }
  return track;
}

std::shared_ptr<ThreadTrack> TrackManager::GetOrCreateThreadTrack(int32_t tid) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<ThreadTrack> track = thread_tracks_[tid];
  if (track == nullptr) {
    track = std::make_shared<ThreadTrack>(time_graph_, tid, app_);
    AddTrack(track);
    thread_tracks_[tid] = track;
    track->SetTrackColor(TimeGraph::GetThreadColor(tid));
    if (tid == orbit_base::kAllThreadsOfAllProcessesTid) {
      track->SetName("All tracepoint events");
      track->SetLabel("All tracepoint events");
    } else if (tid == orbit_base::kAllProcessThreadsTid) {
      // This is the process track.
      const CaptureData& capture_data = app_->GetCaptureData();
      std::string process_name = capture_data.process_name();
      track->SetName(process_name);
      const std::string_view all_threads = " (all_threads)";
      track->SetLabel(process_name.append(all_threads));
      track->SetNumberOfPrioritizedTrailingCharacters(all_threads.size() - 1);
    } else {
      const std::string& thread_name = time_graph_->GetThreadNameFromTid(tid);
      track->SetName(thread_name);
      std::string tid_str = std::to_string(tid);
      std::string track_label = absl::StrFormat("%s [%s]", thread_name, tid_str);
      track->SetNumberOfPrioritizedTrailingCharacters(tid_str.size() + 2);
      track->SetLabel(track_label);
    }
  }
  return track;
}

std::shared_ptr<GpuTrack> TrackManager::GetOrCreateGpuTrack(uint64_t timeline_hash) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<GpuTrack> track = gpu_tracks_[timeline_hash];
  if (track == nullptr) {
    track = std::make_shared<GpuTrack>(time_graph_, string_manager_, timeline_hash, app_);
    std::string timeline = string_manager_->Get(timeline_hash).value_or("");
    std::string label = orbit_gl::MapGpuTimelineToTrackLabel(timeline);
    track->SetName(timeline);
    track->SetLabel(label);
    // This min combine two cases, label == timeline and when label includes timeline
    track->SetNumberOfPrioritizedTrailingCharacters(std::min(label.size(), timeline.size() + 2));
    AddTrack(track);
    gpu_tracks_[timeline_hash] = track;
  }

  return track;
}

GraphTrack* TrackManager::GetOrCreateGraphTrack(const std::string& name) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<GraphTrack> track = graph_tracks_[name];
  if (track == nullptr) {
    track = std::make_shared<GraphTrack>(time_graph_, name);
    track->SetName(name);
    track->SetLabel(name);
    AddTrack(track);
    graph_tracks_[name] = track;
  }

  return track.get();
}

AsyncTrack* TrackManager::GetOrCreateAsyncTrack(const std::string& name) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<AsyncTrack> track = async_tracks_[name];
  if (track == nullptr) {
    track = std::make_shared<AsyncTrack>(time_graph_, name, app_);
    AddTrack(track);
    async_tracks_[name] = track;
  }

  return track.get();
}

std::shared_ptr<FrameTrack> TrackManager::GetOrCreateFrameTrack(const FunctionInfo& function) {
  std::lock_guard<std::recursive_mutex> lock(mutex_);
  std::shared_ptr<FrameTrack> track = frame_tracks_[function.address()];
  if (track == nullptr) {
    track = std::make_shared<FrameTrack>(time_graph_, function, app_);
    // Normally we would call AddTrack(track) here, but frame tracks are removable by users
    // and therefore cannot be simply thrown into the flat vector of tracks.
    sorting_invalidated_ = true;
    frame_tracks_[function.address()] = track;
  }
  return track;
}