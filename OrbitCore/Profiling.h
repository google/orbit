//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Platform.h"
#include "BaseTypes.h"
#include "Utils.h"

//-----------------------------------------------------------------------------
typedef uint64_t TickType;
extern TickType  GFrequency;
extern double    GPeriod;

//-----------------------------------------------------------------------------
void InitProfiling();

//-----------------------------------------------------------------------------
inline TickType OrbitTicks( uint32_t a_Clock = 0 /*CLOCK_REALTIME*/ )
{
#ifdef _WIN32
    UNUSED(a_Clock);
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
#else
    timespec ts;
	clock_gettime(a_Clock, &ts);
	return 1000000000ll * ts.tv_sec + ts.tv_nsec;
#endif
}

#ifdef WIN32_DONT_GO_THROUGH_THIS_CODE // TODO
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
#else
//-----------------------------------------------------------------------------
inline double MicroSecondsFromTicks( TickType a_Start, TickType a_End )
{
    return double((a_End - a_Start))*0.001;
}

//-----------------------------------------------------------------------------
inline TickType TicksFromMicroseconds( double a_Micros )
{
    return (TickType)(a_Micros*1000);
}
#endif
