//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <cstring>
#include <cstdint>

#pragma pack(push, 8)

namespace Orbit {

//-----------------------------------------------------------------------------
struct Data {
  Data() { memset(this, 0, sizeof(*this)); }
  uint64_t m_Time;
  uint64_t m_CallstackHash;
  unsigned long m_ThreadId;
  int m_NumBytes;
  void* m_Data;
};

}  // namespace Orbit

#pragma pack(pop)
