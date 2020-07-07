// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_MEMORY_TRACKER_H_
#define ORBIT_CORE_MEMORY_TRACKER_H_

#include <unordered_map>

#include "Core.h"
#include "ScopeTimer.h"

class Function;
class MemoryTracker {
 public:
  MemoryTracker();
  void ProcessAlloc(const Timer& a_Timer);
  void ProcessFree(const Timer& a_Timer);
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

#endif // ORBIT_CORE_MEMORY_TRACKER_H_
