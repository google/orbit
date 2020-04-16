//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <vector>

#include "Core.h"

#define ORBIT_LOG(msg) GLogger.Log(OrbitLog::Global, msg)
#define ORBIT_LOGV(var) GLogger.Log(OrbitLog::Global, #var, var);
#define ORBIT_VIZ(msg) GLogger.Logf(OrbitLog::Viz, msg)
#define ORBIT_VIZV(var) GLogger.Log(OrbitLog::Viz, #var, var);
#define ORBIT_LOG_DEBUG(msg) GLogger.Log(OrbitLog::Debug, msg)
#define ORBIT_PRINTF(msg) GLogger.Logf(OrbitLog::Viz, msg)
#define ORBIT_LOG_PDB(msg)
#define ORBIT_ERROR GLogger.LogError(__FUNCTION__, __LINE__)

//-----------------------------------------------------------------------------
class OrbitLog {
 public:
  enum Type { Global, Debug, Pdb, Viz, NumLogTypes };

  void Log(const std::string& a_String) {
    m_Entries.push_back(a_String.c_str());
  }
  void Log(const char* a_String) { m_Entries.push_back(a_String); }
  void Log(const wchar_t* a_String) { Log(ws2s(a_String)); }
  void Logf(const std::string& a_String) {
    if (m_Entries.size() == 0)
      m_Entries.push_back(a_String);
    else
      m_Entries[0] += a_String;
  }
  void Clear() { m_Entries.clear(); }
  std::vector<std::string>& GetEntries() { return m_Entries; }

 protected:
  std::vector<std::string> m_Entries;
};

//-----------------------------------------------------------------------------
class Logger {
 public:
  void Log(OrbitLog::Type a_Type, const std::string& a_String) {
    ScopeLock lock(m_Mutexes[a_Type]);
    m_Logs[a_Type].Log(a_String);
  }

  void Logf(OrbitLog::Type a_Type, const std::string& a_String) {
    ScopeLock lock(m_Mutexes[a_Type]);
    m_Logs[a_Type].Logf(a_String);
  }

  void Log(OrbitLog::Type a_Type, const char* a_String) {
    ScopeLock lock(m_Mutexes[a_Type]);
    m_Logs[a_Type].Log(a_String);
  }

  void LogError(const char* function, int line) {
    std::string err = GetLastErrorAsString();
    Log(OrbitLog::Global, absl::StrFormat("Error: %s (%i) LastError: %s",
                                          function, line, err.c_str()));
  }

  template <class T>
  inline void Log(OrbitLog::Type a_Type, const char* a_VarName,
                  const T& a_Value) {
    std::stringstream l_StringStream;
    l_StringStream << a_VarName << " = " << a_Value << std::endl;
    Log(a_Type, l_StringStream.str().c_str());
  }

  void GetLockedLog(OrbitLog::Type a_Type,
                    std::function<void(const std::vector<std::string>)> a_Func,
                    bool a_Clear = false) {
    ScopeLock lock(m_Mutexes[a_Type]);
    a_Func(m_Logs[a_Type].GetEntries());
    if (a_Clear) {
      m_Logs[a_Type].Clear();
    }
  }

  std::vector<std::string> ConsumeEntries(OrbitLog::Type a_Type) {
    ScopeLock lock(m_Mutexes[a_Type]);
    std::vector<std::string> entries = std::move(m_Logs[a_Type].GetEntries());
    return entries;
  }

 protected:
  OrbitLog m_Logs[OrbitLog::NumLogTypes];
  Mutex m_Mutexes[OrbitLog::NumLogTypes];
};

extern Logger GLogger;
