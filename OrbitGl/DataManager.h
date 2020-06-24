// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_MANAGER_H_
#define ORBIT_GL_DATA_MANAGER_H_

#include <thread>

#include "ProcessData.h"
#include "absl/container/flat_hash_map.h"

// This class is responsible for storing and
// navigating data on the client side. Note that
// every method of this class should be called
// on the main thread.
class DataManager final {
 public:
  explicit DataManager(std::thread::id thread_id = std::this_thread::get_id())
      : main_thread_id_(thread_id) {}

  void UpdateProcessInfos(const std::vector<ProcessInfo>& process_infos);
  void UpdateModuleInfos(uint32_t process_id,
                         const std::vector<ModuleInfo>& module_infos);

  ProcessData* GetProcessByPid(uint32_t process_id);
  const std::vector<ModuleData*>& GetModules(uint32_t process_id);
  ModuleData* FindModuleByAddressStart(uint32_t process_id,
                                       uint64_t address_start);

 private:
  const std::thread::id main_thread_id_;
  absl::flat_hash_map<uint32_t, std::unique_ptr<ProcessData>> process_map_;
};

#endif  // ORBIT_GL_DATA_MANAGER_H_
