//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <unordered_map>

#include "Core.h"
#include "ScopeTimer.h"

class Function;
class MemoryTracker {
 public:
  MemoryTracker();
  void ProcessAlloc(const Timer& a_Timer);
  void ProcessFree(const Timer& a_Timer);
  void DumpReport();
  void Clear();

  DWORD64 NumAllocatedBytes() const { return m_NumAllocatedBytes; }
  DWORD64 NumFreedBytes() const { return m_NumFreedBytes; }
  DWORD64 NumLiveBytes() const { return m_NumLiveBytes; }

 protected:
  std::unordered_map<DWORD64, Timer> m_LiveAllocs;
  DWORD64 m_NumAllocatedBytes;
  DWORD64 m_NumFreedBytes;
  DWORD64 m_NumLiveBytes;
};
