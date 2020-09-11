// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CAPTURE_DATA_H_
#define ORBIT_CORE_CAPTURE_DATA_H_

#include <memory>
#include <vector>

#include "CallstackData.h"
#include "OrbitProcess.h"
#include "SamplingProfiler.h"
#include "TracepointCustom.h"
#include "TracepointInfoManager.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class CaptureData {
 public:
  explicit CaptureData(
      int32_t process_id, std::string process_name, std::shared_ptr<Process> process,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
      TracepointInfoSet selected_tracepoints)
      : process_id_{process_id},
        process_name_{std::move(process_name)},
        process_{std::move(process)},
        selected_functions_{std::move(selected_functions)},
        selected_tracepoints_{std::move(selected_tracepoints)},
        callstack_data_(std::make_unique<CallstackData>()),
        selection_callstack_data_(std::make_unique<CallstackData>()),
        tracepoint_info_manager_(std::make_unique<TracepointInfoManager>()) {
    CHECK(process_ != nullptr);
  }
  explicit CaptureData(
      int32_t process_id, std::string process_name, std::shared_ptr<Process> process,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats> functions_stats)
      : process_id_{process_id},
        process_name_{std::move(process_name)},
        process_{std::move(process)},
        selected_functions_{std::move(selected_functions)},
        callstack_data_(std::make_unique<CallstackData>()),
        selection_callstack_data_(std::make_unique<CallstackData>()),
        tracepoint_info_manager_(std::make_unique<TracepointInfoManager>()),
        functions_stats_{std::move(functions_stats)} {
    CHECK(process_ != nullptr);
  }

  explicit CaptureData()
      : process_{std::make_shared<Process>()},
        callstack_data_(std::make_unique<CallstackData>()),
        selection_callstack_data_(std::make_unique<CallstackData>()),
        tracepoint_info_manager_(std::make_unique<TracepointInfoManager>()){};

  // We can not copy the unique_ptr, so we can not copy this object.
  CaptureData& operator=(const CaptureData& other) = delete;
  CaptureData(const CaptureData& other) = delete;

  CaptureData(CaptureData&& other) = default;
  CaptureData& operator=(CaptureData&& other) = default;

  [[nodiscard]] const absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo>&
  selected_functions() const {
    return selected_functions_;
  }

  [[nodiscard]] const orbit_client_protos::FunctionInfo* GetSelectedFunction(
      uint64_t function_address) const;

  [[nodiscard]] int32_t process_id() const { return process_id_; }

  [[nodiscard]] const std::string& process_name() const { return process_name_; }

  [[nodiscard]] const std::chrono::system_clock::time_point& capture_start_time() const {
    return capture_start_time_;
  }

  [[nodiscard]] const absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo>&
  address_infos() const {
    return address_infos_;
  }

  [[nodiscard]] const orbit_client_protos::LinuxAddressInfo* GetAddressInfo(
      uint64_t absolute_address) const;

  void set_address_infos(
      absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> address_infos) {
    address_infos_ = std::move(address_infos);
  }

  void InsertAddressInfo(orbit_client_protos::LinuxAddressInfo address_info);

  [[nodiscard]] const std::string& GetFunctionNameByAddress(uint64_t absolute_address) const;
  [[nodiscard]] const std::string& GetModulePathByAddress(uint64_t absolute_address) const;
  [[nodiscard]] const orbit_client_protos::FunctionInfo* GetFunctionInfoByAddress(
      uint64_t absolute_address) const;

  static const std::string kUnknownFunctionOrModuleName;

  [[nodiscard]] const absl::flat_hash_map<int32_t, std::string>& thread_names() const {
    return thread_names_;
  }

  [[nodiscard]] const std::string& GetThreadName(int32_t thread_id) const {
    static const std::string kEmptyString;
    auto it = thread_names_.find(thread_id);
    return it != thread_names_.end() ? it->second : kEmptyString;
  }

  void set_thread_names(absl::flat_hash_map<int32_t, std::string> thread_names) {
    thread_names_ = std::move(thread_names);
  }

  void AddOrAssignThreadName(int32_t thread_id, std::string thread_name) {
    thread_names_.insert_or_assign(thread_id, std::move(thread_name));
  }

  const absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats>& functions_stats() const {
    return functions_stats_;
  }

  const orbit_client_protos::FunctionStats& GetFunctionStatsOrDefault(
      const orbit_client_protos::FunctionInfo& function) const;

  void UpdateFunctionStats(const orbit_client_protos::FunctionInfo& function,
                           uint64_t elapsed_nanos);

  [[nodiscard]] const CallstackData* GetCallstackData() const { return callstack_data_.get(); };

  void AddUniqueCallStack(CallStack call_stack) {
    callstack_data_->AddUniqueCallStack(std::move(call_stack));
  }

  void AddCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) {
    callstack_data_->AddCallstackEvent(std::move(callstack_event));
  }

  void AddUniqueTracepointEventInfo(uint64_t key,
                                    orbit_grpc_protos::TracepointInfo tracepoint_info) {
    tracepoint_info_manager_->AddUniqueTracepointEventInfo(key, std::move(tracepoint_info));
  }

  void AddTracepointEvent(orbit_client_protos::TracepointEventInfo tracepoint_event_info) {
    tracepoint_info_manager_->AddTracepointEvent(std::move(tracepoint_event_info));
  }

  [[nodiscard]] const CallstackData* GetSelectionCallstackData() const {
    return selection_callstack_data_.get();
  };

  void set_selection_callstack_data(std::unique_ptr<CallstackData> selection_callstack_data) {
    selection_callstack_data_ = std::move(selection_callstack_data);
  }

  [[nodiscard]] const std::shared_ptr<Process>& process() const { return process_; }

  [[nodiscard]] const SamplingProfiler& sampling_profiler() const { return sampling_profiler_; }

  void set_sampling_profiler(SamplingProfiler sampling_profiler) {
    sampling_profiler_ = std::move(sampling_profiler);
  }

 private:
  int32_t process_id_ = -1;
  std::string process_name_;
  std::shared_ptr<Process> process_;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions_;

  TracepointInfoSet selected_tracepoints_;
  // std::unique_ptr<> allows to move and copy CallstackData easier
  // (as CallstackData stores an absl::Mutex inside)
  std::unique_ptr<CallstackData> callstack_data_;
  // selection_callstack_data_ is subset of callstack_data_
  std::unique_ptr<CallstackData> selection_callstack_data_;

  std::unique_ptr<TracepointInfoManager> tracepoint_info_manager_;

  SamplingProfiler sampling_profiler_;

  absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> address_infos_;

  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats> functions_stats_;

  absl::flat_hash_map<int32_t, std::string> thread_names_;

  std::chrono::system_clock::time_point capture_start_time_ = std::chrono::system_clock::now();
};

#endif  // ORBIT_CORE_CAPTURE_DATA_H_
