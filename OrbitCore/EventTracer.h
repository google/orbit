//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once
#include "Core.h"
#include "EventGuid.h"
#include "EventBuffer.h"
#include <evntrace.h>

//-----------------------------------------------------------------------------
class EventTracer
{
public:
    EventTracer();
    ~EventTracer();

    void Init();
    void Start();
    void Stop();

    void CleanupTrace();
    void EventTracerThread();
    void SetSamplingFrequency( float a_Frequency = 1000.f /*Hertz*/ );
    void SetupStackTracing();
    bool IsTracing(){ return m_IsTracing; }
    
    EventBuffer & GetEventBuffer() { return m_EventBuffer; }

protected:
    ULONG64 m_SessionHandle;
    ULONG64 m_TraceHandle;
    std::atomic<bool> m_IsTracing;
    _EVENT_TRACE_PROPERTIES* m_SessionProperties;
    EventBuffer m_EventBuffer;
};

extern EventTracer GEventTracer;
