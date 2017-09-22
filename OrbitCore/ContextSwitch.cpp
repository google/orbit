//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ContextSwitch.h"

//-----------------------------------------------------------------------------
ContextSwitch::ContextSwitch( SwitchType a_Type ) : m_ThreadId( 0 )
                                                  , m_Type( a_Type )
                                                  , m_Time(0)
                                                  , m_ProcessorIndex(0xFF)
                                                  , m_ProcessorNumber(0xFF)
{

}

//-----------------------------------------------------------------------------
ContextSwitch::~ContextSwitch()
{

}