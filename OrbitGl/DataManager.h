// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_MANAGER_H_
#define ORBIT_GL_DATA_MANAGER_H_

#include <thread>

#include "OrbitClientData/FunctionInfoSet.h"
#include "OrbitClientData/ModuleData.h"
#include "OrbitClientData/ProcessData.h"
#include "OrbitClientData/TracepointCustom.h"
#include "OrbitClientData/UserDefinedCaptureData.h"
#include "TextBox.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"
#include "absl/container/node_hash_map.h"
#include "module.pb.h"

// This class is responsible for storing and
// navigating data on the client side. Note that
// every method of this class should be called
// on the main thread.

class DataManager final {
 public:
  explicit DataManager(std::thread::id thread_id = std::this_thread::get_id())
      : main_thread_id_(thread_id) {}

  void UpdateProcessInfos(const std::vector<orbit_grpc_protos::ProcessInfo>& process_infos);

  void SelectFunction(const orbit_client_protos::FunctionInfo& function);
  void DeselectFunction(const orbit_client_protos::FunctionInfo& function);
  void ClearSelectedFunctions();
  void set_visible_functions(absl::flat_hash_set<uint64_t> visible_functions);
  void set_highlighted_function(uint64_t highlighted_function_address);
  void set_selected_thread_id(int32_t thread_id);
  void set_selected_text_box(const TextBox* text_box);
  void set_selected_process(int32_t pid);

  [[nodiscard]] const ProcessData* GetProcessByPid(int32_t process_id) const;
  [[nodiscard]] ProcessData* GetMutableProcessByPid(int32_t process_id);
  [[nodiscard]] bool IsFunctionSelected(const orbit_client_protos::FunctionInfo& function) const;
  [[nodiscard]] std::vector<orbit_client_protos::FunctionInfo> GetSelectedFunctions() const;
  [[nodiscard]] bool IsFunctionVisible(uint64_t function_address) const;
  [[nodiscard]] uint64_t highlighted_function() const;
  [[nodiscard]] int32_t selected_thread_id() const;
  [[nodiscard]] const TextBox* selected_text_box() const;
  [[nodiscard]] const ProcessData* selected_process() const;

  void SelectTracepoint(const orbit_grpc_protos::TracepointInfo& info);
  void DeselectTracepoint(const orbit_grpc_protos::TracepointInfo& info);

  [[nodiscard]] bool IsTracepointSelected(const orbit_grpc_protos::TracepointInfo& info) const;

  [[nodiscard]] const TracepointInfoSet& selected_tracepoints() const;

  void EnableFrameTrack(const orbit_client_protos::FunctionInfo& function);
  void DisableFrameTrack(const orbit_client_protos::FunctionInfo& function);
  [[nodiscard]] bool IsFrameTrackEnabled(const orbit_client_protos::FunctionInfo& function) const;
  void ClearUserDefinedCaptureData();

  void set_user_defined_capture_data(const UserDefinedCaptureData& user_defined_capture_data) {
    user_defined_capture_data_ = user_defined_capture_data;
  }
  [[nodiscard]] const UserDefinedCaptureData& user_defined_capture_data() const {
    return user_defined_capture_data_;
  }
  [[nodiscard]] UserDefinedCaptureData& mutable_user_defined_capture_data() {
    return user_defined_capture_data_;
  }
  static const uint64_t kUnusedHighlightedFunctionAddress;

 private:
  const std::thread::id main_thread_id_;
  // We are sharing pointers to that entries and ensure reference stability by using node_hash_map
  absl::node_hash_map<int32_t, ProcessData> process_map_;
  FunctionInfoSet selected_functions_;
  absl::flat_hash_set<uint64_t> visible_functions_;
  uint64_t highlighted_function_ = kUnusedHighlightedFunctionAddress;

  TracepointInfoSet selected_tracepoints_;

  int32_t selected_thread_id_ = -1;
  const TextBox* selected_text_box_ = nullptr;

  const ProcessData* selected_process_ = nullptr;

  // DataManager needs a copy of this so that we can persist user choices like frame tracks between
  // captures.
  UserDefinedCaptureData user_defined_capture_data_;
};

#endif  // ORBIT_GL_DATA_MANAGER_H_
