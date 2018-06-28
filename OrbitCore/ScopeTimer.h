//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Profiling.h"
#include <string>

#define SCOPE_TIMER_LOG( msg ) LocalScopeTimer ScopeTimer(msg)
extern __declspec(thread) int CurrentDepth;

//-----------------------------------------------------------------------------
class Timer
{
public:
    Timer() : m_TID(0)
            , m_Depth(0)
            , m_SessionID(-1)
            , m_Type(NONE)
            , m_Processor(-1)
            , m_CallstackHash(0)
            , m_FunctionAddress(0)
    {
        m_UserData[0] = 0;
        m_UserData[1] = 0;
    }
    
    void Start();
    void Stop();
    void Reset(){ Stop(); Start(); }
    
    inline double ElapsedMicros() const;
    inline double ElapsedMillis() const;
    inline double ElapsedSeconds() const;

    inline double QueryMillis()  { Stop(); return ElapsedMillis(); }
    inline double QuerySeconds() { Stop(); return ElapsedSeconds(); }

    static inline int GetCurrentDepthTLS() { return CurrentDepth; }
    static inline void ClearThreadDepthTLS() { CurrentDepth = 0; }

    static const int Version = 0;

    enum Type : uint8_t
    {
        NONE,
        CORE_ACTIVITY,
        THREAD_ACTIVITY,
        HIGHLIGHT,
        UNREAL_OBJECT,
        ZONE,
        ALLOC,
        FREE
    };

    Type GetType() const { return m_Type; }
    void SetType( Type a_Type ){ m_Type = a_Type; }
    bool IsType( Type a_Type ) const { return m_Type == a_Type; }

public:

    // Needs to have to exact same layout in win32/x64, debug/release
    int         m_TID;
    int8_t      m_Depth;
    int8_t      m_SessionID;
    Type        m_Type;
    int8_t      m_Processor;
    DWORD64     m_CallstackHash;
    DWORD64     m_FunctionAddress;
    DWORD64     m_UserData[2];
    EpochType   m_Start;
    EpochType   m_End;
};

//-----------------------------------------------------------------------------
class ScopeTimer
{
public:
    ScopeTimer(){}
    ScopeTimer( const char* a_Name );
    ~ScopeTimer();

protected:
    Timer m_Timer;
};

//-----------------------------------------------------------------------------
class LocalScopeTimer
{
public:
    LocalScopeTimer();
    LocalScopeTimer( const std::wstring & a_Message );
    LocalScopeTimer( double* a_Millis );
    ~LocalScopeTimer();
protected:
    Timer        m_Timer;
    double*      m_Millis;
    std::wstring m_Msg;
};

//-----------------------------------------------------------------------------
class ConditionalScopeTimer
{
public:
    ConditionalScopeTimer() : m_Active(false) {}
    ~ConditionalScopeTimer();
    void Start( const char* a_Name );

protected:
    enum { NameSize = 64 };

    Timer m_Timer;
    bool  m_Active;
    char  m_Name[NameSize];
};

//-----------------------------------------------------------------------------
inline double Timer::ElapsedMicros() const
{
    return GetMicroSeconds( m_Start, m_End );
}

//-----------------------------------------------------------------------------
inline double Timer::ElapsedMillis() const
{
    return ElapsedMicros() * 0.001;
}

//-----------------------------------------------------------------------------
inline double Timer::ElapsedSeconds() const
{
    return ElapsedMicros() * 0.000001;
}

