// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <string>
#include <vector>

#include "Threading.h"

#define ORBIT_LOG(msg) GLogger.Log(OrbitLog::Global, msg)
#define ORBIT_LOGV(var) GLogger.Log(OrbitLog::Global, #var, var)
#define ORBIT_VIZV(var) GLogger.Log(OrbitLog::Viz, #var, var)

class OrbitLog {
 public:
  enum Type { Global, Debug, Pdb, Viz, NumLogTypes };

  void Log(const std::string& a_String) { m_Entries.emplace_back(a_String); }
  void Log(const char* a_String) { m_Entries.emplace_back(a_String); }
  void Logf(const std::string& a_String) {
    if (m_Entries.empty())
      m_Entries.push_back(a_String);
    else
      m_Entries[0] += a_String;
  }
  void Clear() { m_Entries.clear(); }
  std::vector<std::string>& GetEntries() { return m_Entries; }

 protected:
  std::vector<std::string> m_Entries;
};

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

  template <class T>
  inline void Log(OrbitLog::Type a_Type, const char* a_VarName,
                  const T& a_Value) {
    std::stringstream l_StringStream;
    l_StringStream << a_VarName << " = " << a_Value << std::endl;
    Log(a_Type, l_StringStream.str().c_str());
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
