//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Profiling.h"

LARGE_INTEGER GFrequency;

//-----------------------------------------------------------------------------
void InitProfiling()
{
    QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>( &GFrequency ) );
}