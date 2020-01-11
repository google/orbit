//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ScopeTimer.h"
#include "TimerManager.h"
#include "Log.h"

thread_local int CurrentDepth = 0;
thread_local int CurrentDepthLocal = 0;

//-----------------------------------------------------------------------------
void Timer::Start()
{
    m_TID = GetCurrentThreadId();
    m_Depth = CurrentDepth++;
    m_SessionID = Message::GSessionID;
    m_Start = OrbitTicks();
}

//-----------------------------------------------------------------------------
void Timer::Stop()
{
    m_End = OrbitTicks();
    --CurrentDepth;
}

//-----------------------------------------------------------------------------
ScopeTimer::ScopeTimer( const char* a_Name )
{
    m_Timer.Start();
}

//-----------------------------------------------------------------------------
ScopeTimer::~ScopeTimer()
{
    m_Timer.Stop();
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer() : m_Millis(nullptr)
{
    ++CurrentDepthLocal;
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer(double* a_Millis) : m_Millis( a_Millis )
{
    ++CurrentDepthLocal;
    m_Timer.Start();
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer( const std::wstring & a_Msg ) : m_Millis(nullptr), m_Msg( a_Msg )
{
    std::wstring tabs;
    for (int i = 0; i < CurrentDepthLocal; ++i)
    {
        tabs += L"  ";
    }
    PRINT( Format(L"%lsStarting %ls...\n", tabs.c_str(), m_Msg.c_str()) );

    ++CurrentDepthLocal;
    m_Timer.Start();
}

//-----------------------------------------------------------------------------
LocalScopeTimer::LocalScopeTimer(const char* a_Message) : LocalScopeTimer(s2ws(a_Message))
{

}

//-----------------------------------------------------------------------------
LocalScopeTimer::~LocalScopeTimer()
{
    m_Timer.Stop();
    --CurrentDepthLocal;

    if( m_Millis )
    {
        *m_Millis = m_Timer.ElapsedMillis();
    }

    if( m_Msg.length() > 0 )
    {
        std::wstring tabs;
        for (int i = 0; i < CurrentDepthLocal; ++i)
        {
            tabs += L"  ";
        }

        PRINT( Format( L"%ls%ls took %f ms.\n", tabs.c_str(), m_Msg.c_str(), m_Timer.ElapsedMillis() ) );
    }
}

//-----------------------------------------------------------------------------
void ConditionalScopeTimer::Start( const char* a_Name )
{
    m_Timer.Start();
    m_Active = true;
}

//-----------------------------------------------------------------------------
ConditionalScopeTimer::~ConditionalScopeTimer()
{
    if( m_Active )
    {
        m_Timer.Stop();
    }
}
