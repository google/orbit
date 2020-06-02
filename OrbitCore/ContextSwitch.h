// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include "Serialization.h"

//-----------------------------------------------------------------------------
#pragma pack(push, 1)
struct ContextSwitch {
 public:
  enum SwitchType : uint8_t { In, Out, Invalid };
  explicit ContextSwitch(SwitchType a_Type = Invalid);
  ~ContextSwitch();

  uint64_t m_Time;
  uint32_t m_ProcessId;
  uint32_t m_ThreadId;
  uint16_t m_ProcessorIndex;
  uint8_t m_ProcessorNumber;
  SwitchType m_Type;

  // TODO: The distinction between m_ProcessorIndex and m_ProcessorNumber is
  //  Windows-specific. Consider removing m_ProcessorNumber.
  // See
  // https://docs.microsoft.com/en-us/windows/win32/api/relogger/ns-relogger-etw_buffer_context
  // and EVENT_HEADER_FLAG_PROCESSOR_INDEX in
  // https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-dtyp/fa4f7836-06ee-4ab6-8688-386a5a85f8c5

  ORBIT_SERIALIZABLE;
};
#pragma pack(pop)
