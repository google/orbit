#include "TimeGraphLayout.h"
#include "Params.h"
#include "Capture.h"
#include <thread>

//-----------------------------------------------------------------------------
TimeGraphLayout::TimeGraphLayout()
{
    m_NumCores = std::thread::hardware_concurrency();

    Reset();

    m_WorldY = 0.f;
    m_TextBoxHeight = 20.f;
    m_CoresHeight = 5.f;
    m_EventTrackHeight = 10.f;
    m_SpaceBetweenCores = 2.f;
    m_SpaceBetweenCoresAndThread = 10.f;
    m_SpaceBetweenTracks = 2.f;
    m_SpaceBetweenTracksAndThread = 2.f;
    m_SpaceBetweenThreadBlocks = 10.f;
};

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadStart()
{
    if( Capture::GHasContextSwitches )
    {
        return m_WorldY - m_NumCores*m_CoresHeight - std::max( m_NumCores - 1, 0 )*m_SpaceBetweenCores - m_SpaceBetweenCoresAndThread;
    }

    return m_WorldY;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetTracksHeight()
{
    return m_NumTracks ? m_NumTracks*m_EventTrackHeight + std::max( m_NumTracks - 1, 0 )*m_SpaceBetweenTracks + m_SpaceBetweenTracksAndThread : 0;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadBlockHeight( ThreadID a_TID )
{
    return GetTracksHeight() + m_ThreadDepths[a_TID] * m_TextBoxHeight;
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::CalculateOffsets()
{
    m_ThreadBlockOffsets.clear();

    m_NumTracks = 0;
    if( m_DrawFileIO ) ++m_NumTracks;
    if( Capture::GHasSamples ) ++m_NumTracks;

    float offset = GetThreadStart();
    for( ThreadID threadID : m_SortedThreadIds )
    {
        m_ThreadBlockOffsets[threadID] = offset;
        offset -= ( GetThreadBlockHeight( threadID ) + m_SpaceBetweenThreadBlocks );
    }
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetCoreOffset( int a_CoreId )
{
    if( Capture::GHasContextSwitches )
    {
        float coreOffset = m_WorldY - m_CoresHeight - a_CoreId * ( m_CoresHeight + m_SpaceBetweenCores );
        return coreOffset;
    }

    return 0.f;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadOffset( ThreadID a_TID, int a_Depth )
{
    return m_ThreadBlockOffsets[a_TID] - GetTracksHeight() - (a_Depth+1)*m_TextBoxHeight;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetThreadBlockStart( ThreadID a_TID )
{
    return m_ThreadBlockOffsets[a_TID];
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetSamplingTrackOffset( ThreadID a_TID )
{
    return Capture::GHasSamples ? m_ThreadBlockOffsets[a_TID] : -1.f;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetFileIOTrackOffset( ThreadID a_TID )
{
    return m_DrawFileIO ? GetSamplingTrackOffset( a_TID ) - m_SpaceBetweenTracks - m_EventTrackHeight : 0.f;
}

//-----------------------------------------------------------------------------
float TimeGraphLayout::GetTotalHeight()
{
    if( m_SortedThreadIds.size() > 0 )
    {
        ThreadID threadId = m_SortedThreadIds.back();
        float offset = GetThreadOffset( threadId, m_ThreadDepths[threadId] );
        return fabs(offset-m_WorldY)+m_TextBoxHeight;
    }

    return GetThreadStart();
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::Reset()
{
    m_DrawFileIO = false;
}

//-----------------------------------------------------------------------------
Color TimeGraphLayout::GetThreadColor( ThreadID a_TID )
{
    auto it = m_ThreadColors.find( a_TID );
    if( it != m_ThreadColors.end() )
    {
        return it->second;
    }

    return Color( 255, 174, 201, 255 );
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::SetThreadColor( ThreadID a_TID, Color a_Color )
{
    m_ThreadColors[a_TID] = a_Color;
}
