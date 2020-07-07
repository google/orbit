// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Callstack.h"

#include "Capture.h"
#include "Serialization.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
std::string CallStack::GetString() {
  std::string callstackString;

  ScopeLock lock(Capture::GTargetProcess->GetDataMutex());
  for (uint32_t i = 0; i < m_Depth; ++i) {
    DWORD64 addr = m_Data[i];
    Function* func =
        Capture::GTargetProcess->GetFunctionFromAddress(addr, false);

    if (func) {
      callstackString += func->PrettyName() + "\n";
    } else {
      callstackString += absl::StrFormat("%" PRIx64 "\n", addr);
    }
  }

  return callstackString;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE(CallStack, 0) {
  ORBIT_NVP_VAL(0, m_Data);
  ORBIT_NVP_VAL(0, m_Hash);
  ORBIT_NVP_VAL(0, m_Depth);
  ORBIT_NVP_VAL(0, m_ThreadId);
}
