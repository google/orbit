//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"

//-----------------------------------------------------------------------------
struct ContextSwitch
{
public:
    enum SwitchType { In, Out, Invalid };
    ContextSwitch( SwitchType a_Type = Invalid );
    ~ContextSwitch();

    unsigned int    m_ThreadId;
    SwitchType      m_Type;
    long long       m_Time;
    USHORT          m_ProcessorIndex;
    UCHAR           m_ProcessorNumber;
};