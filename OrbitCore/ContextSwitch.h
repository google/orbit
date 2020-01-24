//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Serialization.h"

//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct ContextSwitch {
 public:
  enum SwitchType { In, Out, Invalid };
  ContextSwitch(SwitchType a_Type = Invalid);
  ~ContextSwitch();

  uint32_t m_ThreadId;
  SwitchType m_Type;
  uint64_t m_Time;
  uint16_t m_ProcessorIndex;
  uint8_t m_ProcessorNumber;

  ORBIT_SERIALIZABLE;
};
#pragma pack(pop)
// static_assert(sizeof(ContextSwitch) == 19);
// static_assert(offsetof(ContextSwitch, m_ThreadId) == 0);
// static_assert(offsetof(ContextSwitch, m_Type) == 4);
// static_assert(offsetof(ContextSwitch, m_Time) == 8);
// static_assert(offsetof(ContextSwitch, m_ProcessorIndex) == 16);
// static_assert(offsetof(ContextSwitch, m_ProcessorNumber) == 18);