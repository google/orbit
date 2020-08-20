// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_MANAGER_H_
#define ORBIT_GL_DATA_MANAGER_H_

#include <OrbitProcess.h>

#include <thread>

#include "ProcessData.h"
#include "TextBox.h"
#include "absl/container/flat_hash_map.h"
#include "absl/container/flat_hash_set.h"

// This class is responsible for storing and
// navigating data on the client side. Note that
// every method of this class should be called
// on the main thread.
class DataManager final {
 public:
  explicit DataManager(std::thread::id thread_id = std::this_thread::get_id())
      : main_thread_id_(thread_id) {}

  void UpdateProcessInfos(const std::vector<orbit_grpc_protos::ProcessInfo>& process_infos);
  void UpdateModuleInfos(int32_t process_id,
                         const std::vector<orbit_grpc_protos::ModuleInfo>& module_infos);

  void SelectFunction(uint64_t function_address);
  void DeselectFunction(uint64_t function_address);
  void ClearSelectedFunctions();
  void set_selected_functions(absl::flat_hash_set<uint64_t> selected_functions);
  void set_visible_functions(absl::flat_hash_set<uint64_t> visible_functions);
  void set_selected_thread_id(int32_t thread_id);
  void set_selected_text_box(const TextBox* text_box);
  void set_selected_process(std::shared_ptr<Process> process);

  [[nodiscard]] ProcessData* GetProcessByPid(int32_t process_id) const;
  [[nodiscard]] const std::vector<ModuleData*>& GetModules(int32_t process_id) const;
  [[nodiscard]] ModuleData* FindModuleByAddressStart(int32_t process_id,
                                                     uint64_t address_start) const;
  [[nodiscard]] bool IsFunctionSelected(uint64_t function_address) const;
  [[nodiscard]] const absl::flat_hash_set<uint64_t>& selected_functions() const;
  [[nodiscard]] bool IsFunctionVisible(uint64_t function_address) const;
  [[nodiscard]] int32_t selected_thread_id() const;
  [[nodiscard]] const TextBox* selected_text_box() const;
  [[nodiscard]] const std::shared_ptr<Process>& selected_process() const;

 private:
  const std::thread::id main_thread_id_;
  absl::flat_hash_map<int32_t, std::unique_ptr<ProcessData>> process_map_;
  absl::flat_hash_set<uint64_t> selected_functions_;
  absl::flat_hash_set<uint64_t> visible_functions_;
  int32_t selected_thread_id_ = -1;
  const TextBox* selected_text_box_ = nullptr;
  // TODO(kuebler): Remove OrbitProcess class and this member soon.
  std::shared_ptr<Process> selected_process_ = std::make_shared<Process>();
};

#endif  // ORBIT_GL_DATA_MANAGER_H_
