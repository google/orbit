// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CALLSTACK_H_
#define ORBIT_CORE_CALLSTACK_H_

#include <xxhash.h>

#include <string>
#include <vector>

#include "CallstackTypes.h"
#include "SerializationMacros.h"

//-----------------------------------------------------------------------------
struct CallStack {
  CallStack() = default;
  inline CallstackID Hash() {
    if (m_Hash != 0) return m_Hash;
    m_Hash = XXH64(m_Data.data(), m_Depth * sizeof(uint64_t), 0xca1157ac);
    return m_Hash;
  }
  std::string GetString();

  CallstackID m_Hash = 0;
  uint32_t m_Depth = 0;
  ThreadID m_ThreadId = 0;
  std::vector<uint64_t> m_Data;

  ORBIT_SERIALIZABLE;
};

#endif  // ORBIT_CORE_CALLSTACK_H_