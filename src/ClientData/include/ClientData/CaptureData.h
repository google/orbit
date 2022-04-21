// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CLIENT_DATA_CAPTURE_DATA_H_
#define CLIENT_DATA_CAPTURE_DATA_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/synchronization/mutex.h>

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
#include "ClientData/ScopeIdProvider.h"
#include "ClientData/ScopeStats.h"
#include "ClientData/ThreadStateSliceInfo.h"
#include "ClientData/ThreadTrackDataProvider.h"
#include "ClientData/TimerChain.h"
#include "ClientData/TimerData.h"
#include "ClientData/TimerDataManager.h"
#include "ClientData/TimestampIntervalSet.h"
#include "ClientData/TracepointCustom.h"
#include "ClientData/TracepointData.h"
#include "ClientProtos/capture_data.pb.h"
#include "GrpcProtos/capture.pb.h"
#include "GrpcProtos/process.pb.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Logging.h"

namespace orbit_client_data {

class CaptureData {
 public:
  enum class DataSource { kLiveCapture, kLoadedCapture };
  explicit CaptureData(const orbit_grpc_protos::CaptureStarted& capture_started,
                       std::optional<std::filesystem::path> file_path,
                       absl::flat_hash_set<uint64_t> frame_track_function_ids,
                       DataSource data_source);

  // We cannot copy the unique_ptr, so we cannot copy this object.
  CaptureData(const CaptureData& other) = delete;
  CaptureData& operator=(const CaptureData& other) = delete;

  CaptureData(CaptureData&& other) = delete;
  CaptureData& operator=(CaptureData&& other) = delete;

  [[nodiscard]] const orbit_client_data::ProcessData* process() const { return &process_; }
  [[nodiscard]] orbit_client_data::ProcessData* mutable_process() { return &process_; }

  [[nodiscard]] const absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>&
  instrumented_functions() const {
    return instrumented_functions_;
  }

  [[nodiscard]] uint64_t memory_sampling_period_ns() const { return memory_sampling_period_ns_; }
  [[nodiscard]] uint64_t memory_warning_threshold_kb() const {
    return memory_warning_threshold_kb_;
  }
  void set_memory_warning_threshold_kb(uint64_t memory_warning_threshold_kb) {
    memory_warning_threshold_kb_ = memory_warning_threshold_kb;
  }

  [[nodiscard]] const orbit_grpc_protos::InstrumentedFunction* GetInstrumentedFunctionById(
      uint64_t function_id) const;

  [[nodiscard]] uint32_t process_id() const;

  [[nodiscard]] std::string process_name() const;

  [[nodiscard]] const std::optional<std::filesystem::path>& file_path() const { return file_path_; }

  [[nodiscard]] absl::Time capture_start_time() const { return capture_start_time_; }

  [[nodiscard]] const absl::flat_hash_map<uint64_t, LinuxAddressInfo>& address_infos() const {
    return address_infos_;
  }

  [[nodiscard]] const LinuxAddressInfo* GetAddressInfo(uint64_t absolute_address) const;

  void InsertAddressInfo(LinuxAddressInfo address_info);

  static const std::string kUnknownFunctionOrModuleName;

  [[nodiscard]] const absl::flat_hash_map<uint32_t, std::string>& thread_names() const {
    return thread_names_;
  }

  [[nodiscard]] const std::string& GetThreadName(uint32_t thread_id) const {
    static const std::string kEmptyString;
    auto it = thread_names_.find(thread_id);
    return it != thread_names_.end() ? it->second : kEmptyString;
  }

  void AddOrAssignThreadName(uint32_t thread_id, std::string thread_name) {
    thread_names_.insert_or_assign(thread_id, std::move(thread_name));
  }

  [[nodiscard]] bool HasThreadStatesForThread(uint32_t tid) const {
    absl::MutexLock lock{&thread_state_slices_mutex_};
    return thread_state_slices_.count(tid) > 0;
  }

  void AddThreadStateSlice(ThreadStateSliceInfo state_slice) {
    absl::MutexLock lock{&thread_state_slices_mutex_};
    thread_state_slices_[state_slice.tid()].emplace_back(std::move(state_slice));
  }

  // Allows the caller to iterate `action` over all the thread state slices of the specified thread
  // in the time range while holding for the whole time the internal mutex, acquired only once.
  void ForEachThreadStateSliceIntersectingTimeRange(
      uint32_t thread_id, uint64_t min_timestamp, uint64_t max_timestamp,
      const std::function<void(const ThreadStateSliceInfo&)>& action) const;

  [[nodiscard]] const ScopeStats& GetScopeStatsOrDefault(uint64_t scope_id) const;

  void UpdateScopeStats(const TimerInfo& timer_info);
  void AddScopeStats(uint64_t scope_id, ScopeStats stats);

  void OnCaptureComplete();

  [[nodiscard]] const CallstackData& GetCallstackData() const { return callstack_data_; };

  [[nodiscard]] const TracepointInfo* GetTracepointInfo(uint64_t tracepoint_id) const {
    return tracepoint_data_.GetTracepointInfo(tracepoint_id);
  }

  void ForEachTracepointEventOfThreadInTimeRange(
      uint32_t thread_id, uint64_t min_tick, uint64_t max_tick,
      const std::function<void(const TracepointEventInfo&)>& action) const {
    return tracepoint_data_.ForEachTracepointEventOfThreadInTimeRange(thread_id, min_tick, max_tick,
                                                                      action);
  }

  uint32_t GetNumTracepointsForThreadId(uint32_t thread_id) const {
    return tracepoint_data_.GetNumTracepointEventsForThreadId(thread_id);
  }

  void reset_file_path() { file_path_.reset(); }

  void AddUniqueCallstack(uint64_t callstack_id, CallstackInfo callstack) {
    callstack_data_.AddUniqueCallstack(callstack_id, std::move(callstack));
  }

  void AddCallstackEvent(orbit_client_data::CallstackEvent callstack_event) {
    callstack_data_.AddCallstackEvent(std::move(callstack_event));
  }

  void FilterBrokenCallstacks() { callstack_data_.UpdateCallstackTypeBasedOnMajorityStart(); }

  void AddUniqueTracepointInfo(uint64_t tracepoint_id, TracepointInfo tracepoint_info) {
    tracepoint_data_.AddUniqueTracepointInfo(tracepoint_id, std::move(tracepoint_info));
  }

  void AddTracepointEventAndMapToThreads(uint64_t time, uint64_t tracepoint_hash,
                                         uint32_t process_id, uint32_t thread_id, int32_t cpu,
                                         bool is_same_pid_as_target) {
    tracepoint_data_.EmplaceTracepointEvent(time, tracepoint_hash, process_id, thread_id, cpu,
                                            is_same_pid_as_target);
  }

  [[nodiscard]] bool has_post_processed_sampling_data() const {
    return post_processed_sampling_data_.has_value();
  }

  [[nodiscard]] const orbit_client_data::PostProcessedSamplingData& post_processed_sampling_data()
      const {
    ORBIT_CHECK(post_processed_sampling_data_.has_value());
    return post_processed_sampling_data_.value();
  }

  void set_post_processed_sampling_data(
      orbit_client_data::PostProcessedSamplingData post_processed_sampling_data) {
    post_processed_sampling_data_ = std::move(post_processed_sampling_data);
  }

  [[nodiscard]] const orbit_client_data::CallstackData& selection_callstack_data() const {
    ORBIT_CHECK(selection_callstack_data_ != nullptr);
    return *selection_callstack_data_;
  };

  void set_selection_callstack_data(
      std::unique_ptr<orbit_client_data::CallstackData> selection_callstack_data) {
    selection_callstack_data_ = std::move(selection_callstack_data);
  }

  [[nodiscard]] const orbit_client_data::PostProcessedSamplingData&
  selection_post_processed_sampling_data() const {
    ORBIT_CHECK(selection_post_processed_sampling_data_.has_value());
    return selection_post_processed_sampling_data_.value();
  }

  void set_selection_post_processed_sampling_data(
      orbit_client_data::PostProcessedSamplingData selection_post_processed_sampling_data) {
    selection_post_processed_sampling_data_ = std::move(selection_post_processed_sampling_data);
  }

  [[nodiscard]] const orbit_client_data::TimestampIntervalSet& incomplete_data_intervals() const {
    return incomplete_data_intervals_;
  }

  void AddIncompleteDataInterval(uint64_t start_timestamp_ns, uint64_t end_timestamp_ns) {
    incomplete_data_intervals_.Add(start_timestamp_ns, end_timestamp_ns);
  }

  void EnableFrameTrack(uint64_t instrumented_function_id);
  void DisableFrameTrack(uint64_t instrumented_function_id);
  [[nodiscard]] bool IsFrameTrackEnabled(uint64_t instrumented_function_id) const;

  [[nodiscard]] const absl::flat_hash_set<uint64_t>& frame_track_function_ids() const {
    return frame_track_function_ids_;
  }

  [[nodiscard]] std::pair<uint64_t, TimerData*> CreateTimerData() {
    return timer_data_manager_.CreateTimerData();
  }

  [[nodiscard]] ThreadTrackDataProvider* GetThreadTrackDataProvider() const {
    return thread_track_data_provider_.get();
  }

  [[nodiscard]] uint64_t ProvideScopeId(const orbit_client_protos::TimerInfo& timer_info) const;
  [[nodiscard]] std::vector<uint64_t> GetAllProvidedScopeIds() const;
  [[nodiscard]] const ScopeInfo& GetScopeInfo(uint64_t scope_id) const;
  [[nodiscard]] uint64_t FunctionIdToScopeId(uint64_t function_id) const;

  [[nodiscard]] const std::vector<uint64_t>* GetSortedTimerDurationsForScopeId(
      uint64_t scope_id) const;

  // This is a hack allowing for fast jumps to timers. The class should be removed after the way we
  // store the `timer_info`s is refactored.
  class FirstLastMinMaxTimers {
   public:
    explicit FirstLastMinMaxTimers(const TimerInfo* first_timer_observed)
        : first(first_timer_observed),
          last(first_timer_observed),
          min(first_timer_observed),
          max(first_timer_observed) {}

    void Update(const TimerInfo* new_timer);

    const TimerInfo* first;
    const TimerInfo* last;
    const TimerInfo* min;
    const TimerInfo* max;

   private:
    [[nodiscard]] static uint64_t Elapsed(const TimerInfo* timer) {
      return timer->end() - timer->start();
    }
  };

  [[nodiscard]] const FirstLastMinMaxTimers* GetFirstLastMinMaxTimersForScopeId(
      uint64_t scope_id) const;

  // Returns all the timers corresponding to scopes with non-invalid ids
  [[nodiscard]] std::vector<const TimerInfo*> GetAllScopeTimers(
      uint64_t min_tick = std::numeric_limits<uint64_t>::min(),
      uint64_t max_tick = std::numeric_limits<uint64_t>::max()) const;

  [[nodiscard]] std::vector<const TimerInfo*> GetTimersForScope(
      uint64_t scope_id, uint64_t min_tick = std::numeric_limits<uint64_t>::min(),
      uint64_t max_tick = std::numeric_limits<uint64_t>::max()) const;

 private:
  void UpdateTimerDurationsAndFirstLastMinMaxTimers();
  void UpdateFirstLastMinMaxTimers();

  orbit_client_data::ProcessData process_;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction> instrumented_functions_;
  uint64_t memory_sampling_period_ns_;
  uint64_t memory_warning_threshold_kb_;

  CallstackData callstack_data_;
  std::optional<PostProcessedSamplingData> post_processed_sampling_data_;

  // selection_callstack_data_ is subset of callstack_data_.
  // TODO(b/215667641): The callstack selection should be stored in the DataManager.
  std::unique_ptr<orbit_client_data::CallstackData> selection_callstack_data_;
  std::optional<PostProcessedSamplingData> selection_post_processed_sampling_data_;

  TracepointData tracepoint_data_;

  absl::flat_hash_map<uint64_t, LinuxAddressInfo> address_infos_;

  absl::flat_hash_map<uint64_t, ScopeStats> scope_stats_;

  absl::flat_hash_map<uint32_t, std::string> thread_names_;

  // For each thread, assume sorted by timestamp and not overlapping.
  absl::flat_hash_map<uint32_t, std::vector<ThreadStateSliceInfo>> thread_state_slices_
      GUARDED_BY(thread_state_slices_mutex_);
  mutable absl::Mutex thread_state_slices_mutex_;

  // Only access this field from the main thread.
  orbit_client_data::TimestampIntervalSet incomplete_data_intervals_;

  absl::Time capture_start_time_ = absl::Now();

  absl::flat_hash_set<uint64_t> frame_track_function_ids_;

  std::optional<std::filesystem::path> file_path_;

  std::unique_ptr<ScopeIdProvider> scope_id_provider_;

  TimerDataManager timer_data_manager_;
  std::unique_ptr<ThreadTrackDataProvider> thread_track_data_provider_;

  absl::flat_hash_map<uint64_t, std::vector<uint64_t>> scope_id_to_timer_durations_;

  absl::flat_hash_map<uint64_t, FirstLastMinMaxTimers> scope_id_to_first_last_min_max_timers_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CAPTURE_DATA_H_
