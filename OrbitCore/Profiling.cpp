//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Profiling.h"

TickType GFrequency = 1000000000;
double   GPeriod = 0.000000001;


//-----------------------------------------------------------------------------
void InitProfiling()
{
#ifdef _WIN32
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency( &frequency );
    GFrequency = frequency.QuadPart;
    GPeriod = 1.0/(double)GFrequency;
#endif
}
