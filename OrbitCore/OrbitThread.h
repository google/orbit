//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "ScopeTimer.h"
#include "RingBuffer.h"

//-----------------------------------------------------------------------------
class Thread
{
public:
    Thread() : m_TID(0)
             , m_Handle(0)
             , m_Init(false)
             , counter( 0 )
    {
        m_Usage.Fill(0.f);
    }

    float GetUsage();
    void  UpdateUsage();

    DWORD       m_TID;
    HANDLE      m_Handle;
    bool        m_Init;
    FILETIME    m_LastUserTime;
    FILETIME    m_LastKernTime;
    Timer       m_UpdateThreadTimer;
    RingBuffer< float, 32 > m_Usage;
    int counter;
};

