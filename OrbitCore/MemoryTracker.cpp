// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MemoryTracker.h"

//-----------------------------------------------------------------------------
MemoryTracker::MemoryTracker()
    : m_NumAllocatedBytes(0), m_NumFreedBytes(0), m_NumLiveBytes(0) {}

//-----------------------------------------------------------------------------
void MemoryTracker::ProcessAlloc(const Timer& a_Timer) {
  DWORD64 address = a_Timer.m_UserData[0];
  DWORD64 size = a_Timer.m_UserData[1];

  m_LiveAllocs[address] = a_Timer;
  m_NumAllocatedBytes += size;
  m_NumLiveBytes += size;
}

//-----------------------------------------------------------------------------
void MemoryTracker::ProcessFree(const Timer& a_Timer) {
  DWORD64 address = a_Timer.m_UserData[0];
  Timer& timer = m_LiveAllocs[address];
  DWORD64 freedSize = timer.m_UserData[1];

  m_LiveAllocs.erase(address);
  m_NumFreedBytes += freedSize;
  m_NumLiveBytes -= freedSize;
}

//-----------------------------------------------------------------------------
void MemoryTracker::Clear() {
  m_LiveAllocs.clear();
  m_NumAllocatedBytes = 0;
  m_NumFreedBytes = 0;
  m_NumLiveBytes = 0;
}
