// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "ContextSwitch.h"

#include "Serialization.h"

// Context switches are sent as raw bytes, make sure it's the same size
// on every platform.
static_assert(sizeof(ContextSwitch) == 20);

//-----------------------------------------------------------------------------
ContextSwitch::ContextSwitch(SwitchType a_Type)
    : m_Time(0),
      m_ProcessId(0),
      m_ThreadId(0),
      m_ProcessorIndex(0xFF),
      m_ProcessorNumber(0xFF),
      m_Type(a_Type) {}

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
