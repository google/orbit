#include "TimeGraphLayout.h"
#include "Params.h"
#include "Capture.h"
#include "ThreadTrack.h"

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
void TimeGraphLayout::SortTracksByPosition( const ThreadTrackMap& a_ThreadTracks )
{
    std::vector< std::shared_ptr<ThreadTrack> > tracks;

    for( auto& pair : a_ThreadTracks )
    {
        tracks.push_back(pair.second);
    }

    std::sort(tracks.begin(), tracks.end(), 
    [](const std::shared_ptr<ThreadTrack> & a, const std::shared_ptr<ThreadTrack> & b) -> bool
    { 
        return a->GetPos()[1] > b->GetPos()[1];
    });

    m_SortedThreadIds.clear();
    for( auto& track : tracks )
    {
        m_SortedThreadIds.push_back( track->GetID() );
    }
}

//-----------------------------------------------------------------------------
void TimeGraphLayout::CalculateOffsets( const ThreadTrackMap& a_ThreadTracks )
{    
    m_ThreadBlockOffsets.clear();

    m_NumTracks = 0;
    if( m_DrawFileIO ) ++m_NumTracks;
    if( Capture::GHasSamples ) ++m_NumTracks;

    if (!Capture::IsCapturing())
    {
        SortTracksByPosition(a_ThreadTracks);
    }

    float offset = GetThreadStart();
    for( ThreadID threadID : m_SortedThreadIds )
    {
        auto iter = a_ThreadTracks.find(threadID);
        if (iter == a_ThreadTracks.end())
            continue;

        std::shared_ptr<ThreadTrack> track = iter->second;
        m_ThreadBlockOffsets[threadID] = offset;
        float threadBlockHeight = GetTracksHeight() + track->GetDepth() * m_TextBoxHeight;
        offset -= (threadBlockHeight + m_SpaceBetweenThreadBlocks );
    }

    for( auto& pair : a_ThreadTracks )
    {
        auto& track = pair.second;
        if( track.get() && track->IsMoving() )
        {
            m_ThreadBlockOffsets[track->GetID()] = track->GetPos()[1];
        }
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
    auto iter = m_ThreadBlockOffsets.find(a_TID);
    if( iter != m_ThreadBlockOffsets.end() )
        return m_ThreadBlockOffsets[a_TID];
    return -1.f;
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
