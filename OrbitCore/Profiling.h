//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Platform.h"
#include "BaseTypes.h"

//-----------------------------------------------------------------------------
typedef uint64_t TickType;
extern TickType  GFrequency;
extern double    GPeriod;

//-----------------------------------------------------------------------------
void InitProfiling();

//-----------------------------------------------------------------------------
inline TickType OrbitTicks()
{
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

//-----------------------------------------------------------------------------
inline double MicroSecondsFromTicks( TickType a_Start, TickType a_End )
{
    return double(1000000*(a_End - a_Start))*GPeriod;
}

//-----------------------------------------------------------------------------
inline TickType TicksFromMicroseconds( double a_Micros )
{
    return (TickType)(GFrequency*a_Micros*0.000001);
}
