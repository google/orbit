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

  // TODO: The distinction between m_ProcessorIndex and m_ProcessorNumber is
  //  Windows-specific. Consider removing m_ProcessorNumber.
  // See
  // https://docs.microsoft.com/en-us/windows/win32/api/relogger/ns-relogger-etw_buffer_context
  // and EVENT_HEADER_FLAG_PROCESSOR_INDEX in
  // https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/fa4f7836-06ee-4ab6-8688-386a5a85f8c5

  ORBIT_SERIALIZABLE;
};
#pragma pack(pop)
// static_assert(sizeof(ContextSwitch) == 19);
// static_assert(offsetof(ContextSwitch, m_ThreadId) == 0);
// static_assert(offsetof(ContextSwitch, m_Type) == 4);
// static_assert(offsetof(ContextSwitch, m_Time) == 8);
// static_assert(offsetof(ContextSwitch, m_ProcessorIndex) == 16);
// static_assert(offsetof(ContextSwitch, m_ProcessorNumber) == 18);