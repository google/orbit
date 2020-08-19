// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_ORBIT_PROCESS_H_
#define ORBIT_CORE_ORBIT_PROCESS_H_

#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <utility>
#include <vector>

#include "OrbitModule.h"
#include "ScopeTimer.h"
#include "Threading.h"
#include "absl/container/flat_hash_map.h"
#include "capture_data.pb.h"

class Process {
 public:
  Process();

  void AddModule(std::shared_ptr<Module>& a_Module);

  std::map<std::string, std::shared_ptr<Module>>& GetNameToModulesMap() {
    return m_NameToModuleMap;
  }

  void SetName(std::string_view name) { m_Name = name; }
  const std::string& GetName() const { return m_Name; }
  void SetFullPath(std::string_view full_path) { m_FullPath = full_path; }
  const std::string& GetFullPath() const { return m_FullPath; }
  void SetID(int32_t id) { process_id_ = id; }
  [[nodiscard]] int32_t GetId() const { return process_id_; }
  void SetIs64Bit(bool value) { m_Is64Bit = value; }
  bool GetIs64Bit() const { return m_Is64Bit; }

  orbit_client_protos::FunctionInfo* GetFunctionFromAddress(uint64_t address,
                                                            bool a_IsExact = true);
  std::shared_ptr<Module> GetModuleFromAddress(uint64_t a_Address);
  std::shared_ptr<Module> GetModuleFromName(const std::string& a_Name);
  std::shared_ptr<Module> GetModuleFromPath(const std::string& module_path);

  void AddFunction(const std::shared_ptr<orbit_client_protos::FunctionInfo>& function) {
    functions_.push_back(function);
  }
  void AddFunctions(
      const std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>& functions);

  [[nodiscard]] const std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>>&
  GetFunctions() const {
    return functions_;
  }

  Mutex& GetDataMutex() { return data_mutex_; }

 private:
  int32_t process_id_ = -1;

  std::string m_Name;
  std::string m_FullPath;

  bool m_Is64Bit;
  Mutex data_mutex_;

  std::map<uint64_t, std::shared_ptr<Module>> m_Modules;
  // TODO(antonrohr): Change the usage of m_NameToModuleMap to
  //  path_to_module_map_, since the name of a module is not unique
  //  (/usr/lib/libbase.so and /opt/somedir/libbase.so)
  std::map<std::string, std::shared_ptr<Module>> m_NameToModuleMap;
  std::map<std::string, std::shared_ptr<Module>> path_to_module_map_;

  // Transients
  std::vector<std::shared_ptr<orbit_client_protos::FunctionInfo>> functions_;
};

#endif  // ORBIT_CORE_ORBIT_PROCESS_H_
