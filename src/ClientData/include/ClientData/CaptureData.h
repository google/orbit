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
#include "ClientData/FunctionInfoSet.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
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
  explicit CaptureData(orbit_client_data::ModuleManager* module_manager,
                       const orbit_grpc_protos::CaptureStarted& capture_started,
                       std::optional<std::filesystem::path> file_path,
                       absl::flat_hash_set<uint64_t> frame_track_function_ids,
                       DataSource data_source);

  // We can not copy the unique_ptr, so we can not copy this object.
  CaptureData& operator=(const CaptureData& other) = delete;
  CaptureData(const CaptureData& other) = delete;

  CaptureData(CaptureData&& other) = delete;
  CaptureData& operator=(CaptureData&& other) = delete;

  [[nodiscard]] const absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction>&
  instrumented_functions() const {
    return instrumented_functions_;
  }

  [[nodiscard]] const orbit_grpc_protos::InstrumentedFunction* GetInstrumentedFunctionById(
      uint64_t function_id) const;
  [[nodiscard]] std::optional<uint64_t> FindInstrumentedFunctionIdSlow(
      const orbit_client_protos::FunctionInfo& function) const;

  [[nodiscard]] uint32_t process_id() const;

  [[nodiscard]] std::string process_name() const;

  [[nodiscard]] const std::optional<std::filesystem::path>& file_path() const { return file_path_; }

  [[nodiscard]] absl::Time capture_start_time() const { return capture_start_time_; }

  [[nodiscard]] const absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo>&
  address_infos() const {
    return address_infos_;
  }

  [[nodiscard]] const orbit_client_protos::LinuxAddressInfo* GetAddressInfo(
      uint64_t absolute_address) const;

  void InsertAddressInfo(orbit_client_protos::LinuxAddressInfo address_info);

  [[nodiscard]] const std::string& GetFunctionNameByAddress(uint64_t absolute_address) const;
  [[nodiscard]] std::optional<uint64_t> FindFunctionAbsoluteAddressByInstructionAbsoluteAddress(
      uint64_t absolute_address) const;
  [[nodiscard]] const orbit_client_protos::FunctionInfo* FindFunctionByModulePathBuildIdAndOffset(
      const std::string& module_path, const std::string& build_id, uint64_t offset) const;
  [[nodiscard]] const std::string& GetModulePathByAddress(uint64_t absolute_address) const;
  [[nodiscard]] std::optional<std::string> FindModuleBuildIdByAddress(
      uint64_t absolute_address) const;

  [[nodiscard]] const orbit_client_protos::FunctionInfo* FindFunctionByAddress(
      uint64_t absolute_address, bool is_exact) const;
  [[nodiscard]] const orbit_client_data::ModuleData* FindModuleByAddress(
      uint64_t absolute_address) const;
  [[nodiscard]] orbit_client_data::ModuleData* FindMutableModuleByAddress(
      uint64_t absolute_address);

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

  [[nodiscard]] const absl::flat_hash_map<uint32_t,
                                          std::vector<orbit_client_protos::ThreadStateSliceInfo>>&
  thread_state_slices() const {
    return thread_state_slices_;
  }

  [[nodiscard]] bool HasThreadStatesForThread(uint32_t tid) const {
    absl::MutexLock lock{&thread_state_slices_mutex_};
    return thread_state_slices_.count(tid) > 0;
  }

  void AddThreadStateSlice(orbit_client_protos::ThreadStateSliceInfo state_slice) {
    absl::MutexLock lock{&thread_state_slices_mutex_};
    thread_state_slices_[state_slice.tid()].emplace_back(std::move(state_slice));
  }

  // Allows the caller to iterate `action` over all the thread state slices of the specified thread
  // in the time range while holding for the whole time the internal mutex, acquired only once.
  void ForEachThreadStateSliceIntersectingTimeRange(
      uint32_t thread_id, uint64_t min_timestamp, uint64_t max_timestamp,
      const std::function<void(const orbit_client_protos::ThreadStateSliceInfo&)>& action) const;

  [[nodiscard]] const absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats>&
  functions_stats() const {
    return functions_stats_;
  }

  [[nodiscard]] const orbit_client_protos::FunctionStats& GetFunctionStatsOrDefault(
      uint64_t instrumented_function_id) const;

  void UpdateFunctionStats(uint64_t instrumented_function_id, uint64_t elapsed_nanos);
  void AddFunctionStats(uint64_t instrumented_function_id,
                        orbit_client_protos::FunctionStats stats);

  void OnCaptureComplete();

  [[nodiscard]] const CallstackData& GetCallstackData() const { return callstack_data_; };

  [[nodiscard]] orbit_grpc_protos::TracepointInfo GetTracepointInfo(uint64_t key) const {
    return tracepoint_data_.GetTracepointInfo(key);
  }

  void ForEachTracepointEventOfThreadInTimeRange(
      uint32_t thread_id, uint64_t min_tick, uint64_t max_tick,
      const std::function<void(const orbit_client_protos::TracepointEventInfo&)>& action) const {
    return tracepoint_data_.ForEachTracepointEventOfThreadInTimeRange(thread_id, min_tick, max_tick,
                                                                      action);
  }

  uint32_t GetNumTracepointsForThreadId(uint32_t thread_id) const {
    return tracepoint_data_.GetNumTracepointEventsForThreadId(thread_id);
  }

  void reset_file_path() { file_path_.reset(); }

  void AddUniqueCallstack(uint64_t callstack_id, orbit_client_protos::CallstackInfo callstack) {
    callstack_data_.AddUniqueCallstack(callstack_id, std::move(callstack));
  }

  void AddCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) {
    callstack_data_.AddCallstackEvent(std::move(callstack_event));
  }

  void FilterBrokenCallstacks() { callstack_data_.UpdateCallstackTypeBasedOnMajorityStart(); }

  void AddUniqueTracepointEventInfo(uint64_t key,
                                    orbit_grpc_protos::TracepointInfo tracepoint_info) {
    tracepoint_data_.AddUniqueTracepointInfo(key, std::move(tracepoint_info));
  }

  void AddTracepointEventAndMapToThreads(uint64_t time, uint64_t tracepoint_hash,
                                         uint32_t process_id, uint32_t thread_id, int32_t cpu,
                                         bool is_same_pid_as_target) {
    tracepoint_data_.EmplaceTracepointEvent(time, tracepoint_hash, process_id, thread_id, cpu,
                                            is_same_pid_as_target);
  }

  [[nodiscard]] const orbit_client_data::CallstackData* GetSelectionCallstackData() const {
    return selection_callstack_data_.get();
  };

  void set_selection_callstack_data(
      std::unique_ptr<orbit_client_data::CallstackData> selection_callstack_data) {
    selection_callstack_data_ = std::move(selection_callstack_data);
  }

  [[nodiscard]] const orbit_client_data::ProcessData* process() const { return &process_; }
  [[nodiscard]] orbit_client_data::ProcessData* mutable_process() { return &process_; }

  [[nodiscard]] bool has_post_processed_sampling_data() const {
    return post_processed_sampling_data_.has_value();
  }

  [[nodiscard]] const orbit_client_data::PostProcessedSamplingData& post_processed_sampling_data()
      const {
    CHECK(post_processed_sampling_data_.has_value());
    return post_processed_sampling_data_.value();
  }

  void set_post_processed_sampling_data(
      orbit_client_data::PostProcessedSamplingData post_processed_sampling_data) {
    post_processed_sampling_data_ = std::move(post_processed_sampling_data);
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

 private:
  [[nodiscard]] std::optional<uint64_t>
  FindFunctionAbsoluteAddressByInstructionAbsoluteAddressUsingModulesInMemory(
      uint64_t absolute_address) const;
  [[nodiscard]] std::optional<uint64_t>
  FindFunctionAbsoluteAddressByInstructionAbsoluteAddressUsingAddressInfo(
      uint64_t absolute_address) const;

  orbit_client_data::ProcessData process_;
  orbit_client_data::ModuleManager* module_manager_;
  absl::flat_hash_map<uint64_t, orbit_grpc_protos::InstrumentedFunction> instrumented_functions_;

  orbit_client_data::CallstackData callstack_data_;
  // selection_callstack_data_ is subset of callstack_data_
  std::unique_ptr<orbit_client_data::CallstackData> selection_callstack_data_;

  TracepointData tracepoint_data_;

  std::optional<PostProcessedSamplingData> post_processed_sampling_data_;

  absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> address_infos_;

  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats> functions_stats_;

  absl::flat_hash_map<uint32_t, std::string> thread_names_;

  // For each thread, assume sorted by timestamp and not overlapping.
  absl::flat_hash_map<uint32_t, std::vector<orbit_client_protos::ThreadStateSliceInfo>>
      thread_state_slices_ GUARDED_BY(thread_state_slices_mutex_);
  mutable absl::Mutex thread_state_slices_mutex_;

  // Only access this field from the main thread.
  orbit_client_data::TimestampIntervalSet incomplete_data_intervals_;

  absl::Time capture_start_time_ = absl::Now();

  absl::flat_hash_set<uint64_t> frame_track_function_ids_;

  std::optional<std::filesystem::path> file_path_;

  TimerDataManager timer_data_manager_;
  std::unique_ptr<ThreadTrackDataProvider> thread_track_data_provider_;
};

}  // namespace orbit_client_data

#endif  // CLIENT_DATA_CAPTURE_DATA_H_
