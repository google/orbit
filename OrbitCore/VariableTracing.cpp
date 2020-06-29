// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "VariableTracing.h"

//-----------------------------------------------------------------------------
VariableTracing& VariableTracing::Get() {
  static VariableTracing s_Instance;
  return s_Instance;
}

//-----------------------------------------------------------------------------
void VariableTracing::Trace(const char* a_Msg) {
  static size_t maxEntries = 128;

  if (Get().m_Entries.size() < maxEntries) {
    ScopeLock lock(Get().m_Mutex);
    Get().m_Entries.push_back(a_Msg);
  }
}

//-----------------------------------------------------------------------------
void VariableTracing::ProcessCallbacks() {
  ScopeLock lock(Get().m_Mutex);

  for (TraceCallback callback : Get().m_Callbacks) {
    callback(Get().m_Entries);
  }

  Get().m_Entries.clear();
}

//-----------------------------------------------------------------------------
void VariableTracing::AddCallback(TraceCallback a_Callback) {
  // TODO: implement remove callback mechanism
  ScopeLock lock(Get().m_Mutex);

  Get().m_Callbacks.push_back(a_Callback);
}
