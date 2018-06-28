//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <windows.h>
#include "BaseTypes.h"

//-----------------------------------------------------------------------------
typedef uint64_t IntervalType;
typedef uint64_t EpochType;
extern LARGE_INTEGER GFrequency;

//-----------------------------------------------------------------------------
void InitProfiling();

//-----------------------------------------------------------------------------
inline EpochType OrbitTicks()
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

//-----------------------------------------------------------------------------
inline double GetMicroSeconds( EpochType a_Start, EpochType a_End )
{
    IntervalType count = static_cast<IntervalType>( a_End - a_Start );
    count *= IntervalType( 1000000 );
    count /= IntervalType( GFrequency.QuadPart );
    return (double)count;
}

//-----------------------------------------------------------------------------
inline IntervalType TicksFromMicroseconds( double a_Micros )
{
    return GFrequency.QuadPart*(IntervalType)a_Micros / (IntervalType)1000000;
}
