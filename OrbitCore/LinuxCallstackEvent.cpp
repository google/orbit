//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#include "LinuxCallstackEvent.h"
#include "SerializationMacros.h"
#include "Serialization.h"


void LinuxCallstackEvent::Clear()
{
    m_CS.m_Data.clear();
    m_CS.m_Hash = 0;
    m_CS.m_ThreadId = 0;
    m_header = "";
    m_time = 0;
    m_numCallstacks = 0;
}

ORBIT_SERIALIZE( LinuxCallstackEvent, 0 )
{
    ORBIT_NVP_VAL( 0, m_header );
    ORBIT_NVP_VAL( 0, m_time );
    ORBIT_NVP_VAL( 0, m_numCallstacks );
    ORBIT_NVP_VAL( 0, m_CS );
}