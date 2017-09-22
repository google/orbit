//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

#pragma pack(push, 8)

namespace Orbit {

//-----------------------------------------------------------------------------
struct Data
{
    Data() { memset(this, 0, sizeof(*this)); }
    unsigned __int64 m_Time;
    unsigned __int64 m_CallstackHash;
    unsigned long    m_ThreadId;
    int              m_NumBytes;
    void*            m_Data;
};

}

#pragma pack(pop)