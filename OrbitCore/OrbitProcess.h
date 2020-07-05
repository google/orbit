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

#ifdef __linux__
#include "LinuxUtils.h"
#endif

class Function;
class Type;
class Variable;
class Thread;
class Session;
struct Module;

//-----------------------------------------------------------------------------
class Process {
 public:
  Process();
  ~Process();

  void LoadDebugInfo();
  void UpdateCpuTime();
  bool IsElevated() const { return m_IsElevated; }
  void SetThreadName(int32_t thread_id, std::string thread_name) {
    m_ThreadNames[thread_id] = std::move(thread_name);
  }
  std::string GetThreadNameFromTID(DWORD a_ThreadId) {
    return m_ThreadNames[a_ThreadId];
  }
  void AddModule(std::shared_ptr<Module>& a_Module);
  void FindPdbs(const std::vector<std::string>& a_SearchLocations);

  static bool IsElevated(HANDLE a_Process);

#ifdef _WIN32
  static bool SetPrivilege(LPCTSTR a_Name, bool a_Enable);
#endif

  std::map<uint64_t, std::shared_ptr<Module>>& GetModules() {
    return m_Modules;
  }

  // This method is currently used by client
  std::vector<std::shared_ptr<Module>> GetModulesAsVector() const {
    std::vector<std::shared_ptr<Module>> result;
    for (auto& it : m_Modules) {
      result.push_back(it.second);
    }
    return result;
  }

  std::map<std::string, std::shared_ptr<Module>>& GetNameToModulesMap() {
    return m_NameToModuleMap;
  }
  std::shared_ptr<Module> FindModule(const std::string& a_ModuleName);

  void SetName(std::string_view name) { m_Name = name; }
  const std::string& GetName() const { return m_Name; }
  void SetFullPath(std::string_view full_path) { m_FullPath = full_path; }
  const std::string& GetFullPath() const { return m_FullPath; }
  void SetCmdLine(std::string_view cmd_line) { m_CmdLine = cmd_line; }
  const std::string& GetCmdLine() const { return m_CmdLine; }
  int32_t GetID() const { return m_ID; }
  double GetCpuUsage() const { return m_CpuUsage; }
  HANDLE GetHandle() const { return m_Handle; }
  bool GetIs64Bit() const { return m_Is64Bit; }
  size_t NumModules() const { return m_Modules.size(); }
  bool GetIsRemote() const { return m_IsRemote; }
  void SetIsRemote(bool val) { m_IsRemote = val; }
  void SetCpuUsage(float a_Usage) { m_CpuUsage = a_Usage; }

  Function* GetFunctionFromAddress(uint64_t address, bool a_IsExact = true);
  std::shared_ptr<Module> GetModuleFromAddress(uint64_t a_Address);
  std::shared_ptr<Module> GetModuleFromName(const std::string& a_Name);
  std::shared_ptr<Module> GetModuleFromPath(const std::string& module_path);

  bool LineInfoFromAddress(uint64_t a_Address, struct LineInfo& o_LineInfo);

  void LoadSession(const Session& a_Session);
  void SavePreset();

  void AddFunction(const std::shared_ptr<Function>& function) {
    m_Functions.push_back(function);
  }

  void SetFunctions(const std::vector<std::shared_ptr<Function>>& functions) {
    m_Functions = functions;
  }

  const std::vector<std::shared_ptr<Function>>& GetFunctions() const {
    return m_Functions;
  }

  std::vector<Type*>& GetTypes() { return m_Types; }
  std::vector<Variable*>& GetGlobals() { return m_Globals; }

  void AddWatchedVariable(std::shared_ptr<Variable> a_Variable) {
    m_WatchedVariables.push_back(a_Variable);
  }
  const std::vector<std::shared_ptr<Variable>>& GetWatchedVariables() {
    return m_WatchedVariables;
  }
  void RefreshWatchedVariables();
  void ClearWatchedVariables();

  void AddType(Type& a_Type);
  void SetID(int32_t id);

  Mutex& GetDataMutex() { return m_DataMutex; }

  uint64_t GetOutputDebugStringAddress();
  uint64_t GetRaiseExceptionAddress();

 private:
  void ClearTransients();

  int32_t m_ID;
  HANDLE m_Handle;
  bool m_IsElevated;

  std::string m_Name;
  std::string m_FullPath;
  std::string m_CmdLine;

  FILETIME m_LastUserTime;
  FILETIME m_LastKernTime;
  double m_CpuUsage;
  Timer m_UpdateCpuTimer;
  bool m_Is64Bit;
  bool m_DebugInfoLoaded;
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
  std::vector<Type*> m_Types;
  std::vector<Variable*> m_Globals;
  std::vector<std::shared_ptr<Variable>> m_WatchedVariables;

  std::unordered_set<uint64_t> m_UniqueTypeHash;

  friend class TestRemoteMessages;
};

#endif // ORBIT_CORE_ORBIT_PROCESS_H_
