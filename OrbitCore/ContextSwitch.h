//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Serialization.h"

//-----------------------------------------------------------------------------
struct ContextSwitch
{
public:
    enum SwitchType { In, Out, Invalid };
    ContextSwitch( SwitchType a_Type = Invalid );
    ~ContextSwitch();

    uint32_t    m_ThreadId;
    SwitchType  m_Type;
    uint64_t    m_Time;
    uint16_t    m_ProcessorIndex;
    uint8_t     m_ProcessorNumber;

    ORBIT_SERIALIZABLE;
};