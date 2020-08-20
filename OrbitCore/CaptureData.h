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
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class CaptureData {
 public:
  explicit CaptureData(
      int32_t process_id, std::string process_name, std::shared_ptr<Process> process,
      absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions)
      : process_id_{process_id},
        process_name_{std::move(process_name)},
        process_{std::move(process)},
        selected_functions_{std::move(selected_functions)},
        callstack_data_(std::make_unique<CallstackData>()),
        selection_callstack_data_(std::make_unique<CallstackData>()),
        sampling_profiler_{std::make_shared<SamplingProfiler>(process_)} {
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
        sampling_profiler_{std::make_shared<SamplingProfiler>(process_)},
        functions_stats_{std::move(functions_stats)} {
    CHECK(process_ != nullptr);
  }

  explicit CaptureData()
      : process_{std::make_shared<Process>()},
        callstack_data_(std::make_unique<CallstackData>()),
        selection_callstack_data_(std::make_unique<CallstackData>()),
        sampling_profiler_{std::make_shared<SamplingProfiler>(process_)} {};
  CaptureData(const CaptureData& other) = delete;
  // We can not copy the unique_ptr, so we can not copy this object.
  CaptureData& operator=(const CaptureData& other) = delete;
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

  void set_address_infos(
      absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> address_infos) {
    address_infos_ = std::move(address_infos);
  }

  [[nodiscard]] orbit_client_protos::LinuxAddressInfo* GetAddressInfo(uint64_t address);

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

  const absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats>& functions_stats() {
    return functions_stats_;
  }

  const orbit_client_protos::FunctionStats& GetFunctionStatsOrDefault(uint64_t function_address);

  void UpdateFunctionStats(orbit_client_protos::FunctionInfo* func,
                           const orbit_client_protos::TimerInfo& timer_info);

  [[nodiscard]] const CallstackData* GetCallstackData() const { return callstack_data_.get(); };

  void AddUniqueCallStack(CallStack call_stack) {
    callstack_data_->AddUniqueCallStack(std::move(call_stack));
  }

  void AddCallstackEvent(orbit_client_protos::CallstackEvent callstack_event) {
    callstack_data_->AddCallstackEvent(std::move(callstack_event));
  }

  [[nodiscard]] const CallstackData* GetSelectionCallstackData() const {
    return selection_callstack_data_.get();
  };

  void SetSelectionCallstackData(std::unique_ptr<CallstackData> selection_callstack_data) {
    selection_callstack_data_ = std::move(selection_callstack_data);
  }

  [[nodiscard]] const std::shared_ptr<Process>& process() const { return process_; }

  [[nodiscard]] const std::shared_ptr<SamplingProfiler>& sampling_profiler() const {
    return sampling_profiler_;
  }

 private:
  int32_t process_id_ = -1;
  std::string process_name_;
  std::shared_ptr<Process> process_;
  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionInfo> selected_functions_;
  // std::unique_ptr<> allows to move and copy CallstackData easier
  // (as CallstackData stores an absl::Mutex inside)
  std::unique_ptr<CallstackData> callstack_data_;
  // selection_callstack_data_ is subset of callstack_data_
  std::unique_ptr<CallstackData> selection_callstack_data_;

  // TODO(kuebler): Make this value type as soon as UI/SamplingReport does not need to modify it
  //  anymore.
  std::shared_ptr<SamplingProfiler> sampling_profiler_;

  absl::flat_hash_map<uint64_t, orbit_client_protos::LinuxAddressInfo> address_infos_;

  absl::flat_hash_map<uint64_t, orbit_client_protos::FunctionStats> functions_stats_;

  absl::flat_hash_map<int32_t, std::string> thread_names_;

  std::chrono::system_clock::time_point capture_start_time_ = std::chrono::system_clock::now();
};

#endif  // ORBIT_CORE_CAPTURE_DATA_H_
