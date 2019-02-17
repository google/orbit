//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#pragma pack(push, 8)

namespace Orbit {

//-----------------------------------------------------------------------------
struct UserData
{
    UserData() { memset(this, 0, sizeof(*this)); }
    uint64_t        m_Time;
    uint64_t        m_CallstackHash;
    unsigned long   m_ThreadId;
    int             m_NumBytes;
    void*           m_Data;
};

}

#pragma pack(pop)