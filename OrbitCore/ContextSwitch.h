//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

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
    unsigned short  m_ProcessorIndex;
    unsigned char   m_ProcessorNumber;
};