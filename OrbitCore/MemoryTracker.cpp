// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "MemoryTracker.h"

#include "Callstack.h"
#include "Capture.h"
#include "Log.h"
#include "OrbitProcess.h"
#include "absl/strings/str_format.h"

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

  /*PRINT_VAR((void*)address);
  PRINT_VAR(size);*/
}

//-----------------------------------------------------------------------------
void MemoryTracker::ProcessFree(const Timer& a_Timer) {
  DWORD64 address = a_Timer.m_UserData[0];
  Timer& timer = m_LiveAllocs[address];
  DWORD64 freedSize = timer.m_UserData[1];

  m_LiveAllocs.erase(address);
  m_NumFreedBytes += freedSize;
  m_NumLiveBytes -= freedSize;

  // PRINT_VAR(freedSize);
}

//-----------------------------------------------------------------------------
void MemoryTracker::DumpReport() {
  DWORD64 NumLiveBytes = 0;
  std::unordered_map<CallstackID, DWORD64> callstackToBytes;
  for (auto& pair : m_LiveAllocs) {
    Timer& timer = pair.second;
    DWORD64 size = timer.m_UserData[1];
    CallstackID id = timer.m_CallstackHash;
    callstackToBytes[id] += size;
    NumLiveBytes += size;
  }

  std::multimap<DWORD64, CallstackID> bytesToCallstack;
  for (auto& pair : callstackToBytes) {
    bytesToCallstack.insert(std::make_pair(pair.second, pair.first));
  }

  if (m_NumAllocatedBytes) {
    ORBIT_VIZ(absl::StrFormat("NumLiveBytes: %llu\n", NumLiveBytes));
  }

  for (auto rit = bytesToCallstack.rbegin(); rit != bytesToCallstack.rend();
       ++rit) {
    CallstackID id = rit->second;
    std::shared_ptr<CallStack> callstack = Capture::GetCallstack(id);

    DWORD64 numBytes = rit->first;
    DWORD64 cid = id;
    std::string msg = absl::StrFormat("Callstack[%llu] allocated %llu bytes\n",
                                      cid, numBytes);
    ORBIT_VIZ(msg);
    if (callstack) {
      ORBIT_VIZ(callstack->GetString());
    }
    ORBIT_VIZ("\n\n");
  }
}

//-----------------------------------------------------------------------------
void MemoryTracker::Clear() {
  m_LiveAllocs.clear();
  m_NumAllocatedBytes = 0;
  m_NumFreedBytes = 0;
  m_NumLiveBytes = 0;
}
