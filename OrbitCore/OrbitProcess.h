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

#include "BaseTypes.h"
#include "LinuxAddressInfo.h"
#include "ScopeTimer.h"
#include "Threading.h"
#include "absl/container/flat_hash_map.h"

class Function;
struct Module;

//-----------------------------------------------------------------------------
class Process {
 public:
  Process();

  void SetThreadName(int32_t thread_id, std::string thread_name) {
    m_ThreadNames[thread_id] = std::move(thread_name);
  }
  std::string GetThreadNameFromTID(DWORD a_ThreadId) {
    return m_ThreadNames[a_ThreadId];
  }
  void AddModule(std::shared_ptr<Module>& a_Module);

  std::map<std::string, std::shared_ptr<Module>>& GetNameToModulesMap() {
    return m_NameToModuleMap;
  }

  void SetName(std::string_view name) { m_Name = name; }
  const std::string& GetName() const { return m_Name; }
  void SetFullPath(std::string_view full_path) { m_FullPath = full_path; }
  const std::string& GetFullPath() const { return m_FullPath; }
  void SetID(int32_t id) { m_ID = id; }
  int32_t GetID() const { return m_ID; }
  bool GetIs64Bit() const { return m_Is64Bit; }
  bool GetIsRemote() const { return m_IsRemote; }
  void SetIsRemote(bool val) { m_IsRemote = val; }

  Function* GetFunctionFromAddress(uint64_t address, bool a_IsExact = true);
  std::shared_ptr<Module> GetModuleFromAddress(uint64_t a_Address);
  std::shared_ptr<Module> GetModuleFromName(const std::string& a_Name);
  std::shared_ptr<Module> GetModuleFromPath(const std::string& module_path);

  bool LineInfoFromAddress(uint64_t a_Address, struct LineInfo& o_LineInfo);

  void AddFunction(const std::shared_ptr<Function>& function) {
    m_Functions.push_back(function);
  }

  const std::vector<std::shared_ptr<Function>>& GetFunctions() const {
    return m_Functions;
  }

  Mutex& GetDataMutex() { return m_DataMutex; }

 private:
  void ClearTransients();

  int32_t m_ID;

  std::string m_Name;
  std::string m_FullPath;

  bool m_Is64Bit;
  bool m_IsRemote;
  Mutex m_DataMutex;

  std::map<uint64_t, std::shared_ptr<Module>> m_Modules;
  // TODO(antonrohr) change the usage of m_NameToModuleMap to
  // path_to_module_map_, since the name of a module is not unique
  // (/usr/lib/libbase.so and /opt/somedir/libbase.so)
  std::map<std::string, std::shared_ptr<Module>> m_NameToModuleMap;
  std::map<std::string, std::shared_ptr<Module>> path_to_module_map_;
  std::map<int32_t, std::string> m_ThreadNames;

  // Transients
  std::vector<std::shared_ptr<Function>> m_Functions;
};

#endif  // ORBIT_CORE_ORBIT_PROCESS_H_
