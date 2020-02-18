//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "Core.h"
#include "OrbitFunction.h"
#include "ScopeTimer.h"

class Systrace {
 public:
  Systrace(const char* a_FilePath, uint64_t a_TimeOffsetNs = 0);
  const std::vector<Timer>& GetTimers() const { return m_Timers; }
  const std::unordered_map<uint64_t, std::string>& GetStrings() const {
    return m_StringMap;
  }
  const std::string& GetFunctionName(uint64_t a_ID) const;
  const std::unordered_map<DWORD, std::string>& GetThreadNames() const {
    return m_ThreadNames;
  }
  std::vector<Function>& GetFunctions() { return m_Functions; }
  uint64_t GetMinTime() const { return m_MinTime; }
  uint64_t GetMaxTime() const { return m_MaxTime; }
  uint64_t GetTimeRange() const { return m_MaxTime - m_MinTime; }
  const std::string& GetName() const { return m_Name; }

 protected:
  DWORD GetThreadId(const std::string& a_ThreadName);
  uint64_t ProcessString(const std::string& a_String);
  uint64_t ProcessFunctionName(const std::string& a_String);
  void UpdateMinMax(const Timer& a_Timer);

 private:
  std::vector<Timer> m_Timers;
  std::map<std::string, std::vector<Timer>> m_TimerStacks;
  std::map<std::string, DWORD> m_ThreadIDs;
  std::unordered_map<DWORD, std::string> m_ThreadNames;
  std::unordered_map<uint64_t, std::string> m_StringMap;
  std::vector<Function> m_Functions;
  std::unordered_map<uint64_t, Function*> m_FunctionMap;

  DWORD m_ThreadCount = 0;
  DWORD m_StringCount = 0;

  std::string m_Name;

  uint64_t m_TimeOffsetNs = 0;
  uint64_t m_MinTime = 0xFFFFFFFFFFFFFFFF;
  uint64_t m_MaxTime = 0;
};

class SystraceManager {
 public:
  static SystraceManager& Get();

  void Clear();

  void Add(std::shared_ptr<Systrace> a_Systrace);
  const std::string& GetFunctionName(uint64_t a_ID) const;
  bool IsEmpty() { return m_Systraces.size() == 0; }
  uint32_t GetNewThreadID() { return m_ThreadCount++; }
  void Dump() const;

 protected:
  std::vector<std::shared_ptr<Systrace>> m_Systraces;
  uint32_t m_ThreadCount = 0;
};