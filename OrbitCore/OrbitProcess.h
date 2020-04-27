//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <map>
#include <memory>
#include <set>
#include <unordered_set>
#include <vector>

#include "BaseTypes.h"
#include "DiaManager.h"
#include "LinuxAddressInfo.h"
#include "ScopeTimer.h"
#include "SerializationMacros.h"
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
struct ModuleDebugInfo;
struct IDiaSymbol;

//-----------------------------------------------------------------------------
class Process {
 public:
  Process();
  Process(DWORD a_ID);
  ~Process();

  void Init();
  void LoadDebugInfo();
  void ListModules();
  void EnumerateThreads();
  void UpdateCpuTime();
  void UpdateThreadUsage();
  void SortThreadsByUsage();
  void SortThreadsById();
  bool IsElevated() const { return m_IsElevated; }
  bool HasThread(DWORD a_ThreadId) const {
    return m_ThreadIds.find(a_ThreadId) != m_ThreadIds.end();
  }
  void AddThreadId(DWORD a_ThreadId) { m_ThreadIds.insert(a_ThreadId); }
  void RemoveThreadId(DWORD a_ThreadId) { m_ThreadIds.erase(a_ThreadId); };
  void SetThreadName(DWORD a_ThreadId, std::string a_Name) {
    m_ThreadNames[a_ThreadId] = a_Name;
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
  std::map<std::string, std::shared_ptr<Module>>& GetNameToModulesMap() {
    return m_NameToModuleMap;
  }
  std::shared_ptr<Module> FindModule(const std::string& a_ModuleName);

  const std::string& GetName() const { return m_Name; }
  const std::string& GetFullPath() const { return m_FullPath; }
  const std::string& GetCmdLine() const { return m_CmdLine; }
  DWORD GetID() const { return m_ID; }
  double GetCpuUsage() const { return m_CpuUsage; }
  HANDLE GetHandle() const { return m_Handle; }
  bool GetIs64Bit() const { return m_Is64Bit; }
  uint32_t NumModules() const { return (uint32_t)m_Modules.size(); }
  bool GetIsRemote() const { return m_IsRemote; }
  void SetIsRemote(bool val) { m_IsRemote = val; }
  void SetCpuUsage(float a_Usage) { m_CpuUsage = a_Usage; }

  Function* GetFunctionFromAddress(uint64_t address, bool a_IsExact = true);
  std::shared_ptr<Module> GetModuleFromAddress(uint64_t a_Address);
  std::shared_ptr<Module> GetModuleFromName(const std::string& a_Name);

#ifdef _WIN32
  std::shared_ptr<OrbitDiaSymbol> SymbolFromAddress(DWORD64 a_Address);
#endif
  LinuxAddressInfo* GetLinuxAddressInfo(uint64_t a_Address) {
    if (m_AddressInfos.contains(a_Address)) {
      return &m_AddressInfos.at(a_Address);
    } else {
      return nullptr;
    }
  }
  void AddAddressInfo(LinuxAddressInfo address_info);
  bool HasAddressInfo(uint64_t address) const {
    return m_AddressInfos.contains(address);
  }

  bool LineInfoFromAddress(DWORD64 a_Address, struct LineInfo& o_LineInfo);

  void LoadSession(const Session& a_Session);
  void SaveSession();

  void AddFunction(const std::shared_ptr<Function>& function) {
    m_Functions.push_back(function);
  }
  const std::vector<std::shared_ptr<Function>>& GetFunctions() const {
    return m_Functions;
  }

  std::vector<Type*>& GetTypes() { return m_Types; }
  std::vector<Variable*>& GetGlobals() { return m_Globals; }
  std::vector<std::shared_ptr<Thread>>& GetThreads() { return m_Threads; }

  void AddWatchedVariable(std::shared_ptr<Variable> a_Variable) {
    m_WatchedVariables.push_back(a_Variable);
  }
  const std::vector<std::shared_ptr<Variable>>& GetWatchedVariables() {
    return m_WatchedVariables;
  }
  void RefreshWatchedVariables();
  void ClearWatchedVariables();

  void AddType(Type& a_Type);
  void SetID(DWORD a_ID);

  Mutex& GetDataMutex() { return m_DataMutex; }

  DWORD64 GetOutputDebugStringAddress();
  DWORD64 GetRaiseExceptionAddress();

  void FindCoreFunctions();

  std::string m_Name;
  std::string m_FullPath;
  std::string m_CmdLine;

  ORBIT_SERIALIZABLE;

 protected:
  void ClearTransients();

 private:
  DWORD m_ID;
  HANDLE m_Handle;
  bool m_IsElevated;

  FILETIME m_LastUserTime;
  FILETIME m_LastKernTime;
  double m_CpuUsage;
  Timer m_UpdateCpuTimer;
  bool m_Is64Bit;
  bool m_DebugInfoLoaded;
  bool m_IsRemote;
  Mutex m_DataMutex;

  std::map<uint64_t, std::shared_ptr<Module>> m_Modules;
  std::map<std::string, std::shared_ptr<Module>> m_NameToModuleMap;
  std::vector<std::shared_ptr<Thread>> m_Threads;
  std::unordered_set<uint32_t> m_ThreadIds;
  std::map<uint32_t, std::string> m_ThreadNames;

  absl::flat_hash_map<uint64_t, LinuxAddressInfo> m_AddressInfos;

  // Transients
  std::vector<std::shared_ptr<Function>> m_Functions;
  std::vector<Type*> m_Types;
  std::vector<Variable*> m_Globals;
  std::vector<std::shared_ptr<Variable>> m_WatchedVariables;

  std::unordered_set<uint64_t> m_UniqueTypeHash;

  friend class TestRemoteMessages;
};
