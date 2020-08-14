// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_MANAGER_H_
#define ORBIT_GL_DATA_MANAGER_H_

#include <thread>

#include "ProcessData.h"
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

  void UpdateProcessInfos(const std::vector<ProcessInfo>& process_infos);
  void UpdateModuleInfos(int32_t process_id,
                         const std::vector<ModuleInfo>& module_infos);

  void SelectFunction(uint64_t function_address);
  void UnSelectFunction(uint64_t function_address);
  void ClearSelectedFunctions();
  void set_selected_functions(absl::flat_hash_set<uint64_t> selected_functions);

  [[nodiscard]] ProcessData* GetProcessByPid(int32_t process_id) const;
  [[nodiscard]] const std::vector<ModuleData*>& GetModules(
      int32_t process_id) const;
  [[nodiscard]] ModuleData* FindModuleByAddressStart(
      int32_t process_id, uint64_t address_start) const;
  [[nodiscard]] bool IsFunctionSelected(uint64_t function_address) const;
  [[nodiscard]] const absl::flat_hash_set<uint64_t>& selected_functions() const;

 private:
  const std::thread::id main_thread_id_;
  absl::flat_hash_map<int32_t, std::unique_ptr<ProcessData>> process_map_;
  absl::flat_hash_set<uint64_t> selected_functions_;
};

#endif  // ORBIT_GL_DATA_MANAGER_H_
