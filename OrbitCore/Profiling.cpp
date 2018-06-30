//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Profiling.h"

TickType GFrequency;
double   GPeriod;


//-----------------------------------------------------------------------------
void InitProfiling()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency );
    GFrequency = frequency.QuadPart;
    GPeriod = 1.0/(double)GFrequency;
}
