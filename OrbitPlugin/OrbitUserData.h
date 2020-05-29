// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <cstdint>
#include <cstring>

#pragma pack(push, 8)

namespace Orbit {

//-----------------------------------------------------------------------------
struct UserData {
  UserData() { std::memset(this, 0, sizeof(*this)); }
  uint64_t m_Time;
  uint64_t m_CallstackHash;
  unsigned long m_ThreadId;
  int m_NumBytes;
  void* m_Data;
};

}  // namespace Orbit

#pragma pack(pop)