//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ContextSwitch.h"

#include "Serialization.h"

//-----------------------------------------------------------------------------
ContextSwitch::ContextSwitch(SwitchType a_Type)
    : m_ProcessId(0),
      m_ThreadId(0),
      m_Type(a_Type),
      m_Time(0),
      m_ProcessorIndex(0xFF),
      m_ProcessorNumber(0xFF) {}

//-----------------------------------------------------------------------------
ContextSwitch::~ContextSwitch() = default;

ORBIT_SERIALIZE(ContextSwitch, 0) {
  ORBIT_NVP_VAL(0, m_ProcessId);
  ORBIT_NVP_VAL(0, m_ThreadId);
  ORBIT_NVP_VAL(0, m_Type);
  ORBIT_NVP_VAL(0, m_Time);
  ORBIT_NVP_VAL(0, m_ProcessorIndex);
  ORBIT_NVP_VAL(0, m_ProcessorNumber);
}