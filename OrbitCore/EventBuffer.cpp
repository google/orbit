//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "EventBuffer.h"
#include "Serialization.h"

//-----------------------------------------------------------------------------
void EventBuffer::Print()
{
    PRINT("Orbit Callstack Events:");

    size_t numCallstacks = 0;
    for( auto & pair : m_CallstackEvents )
    {
        ThreadID threadID = pair.first;
        std::map< long long, CallstackEvent > & callstacks = pair.second;
        numCallstacks += callstacks.size();
    }

    PRINT_VAR( numCallstacks );

    for( auto & pair : m_CallstackEvents )
    {
        ThreadID threadID = pair.first;
        std::map< long long, CallstackEvent > & callstacks = pair.second;
        PRINT_VAR( threadID );
        PRINT_VAR( callstacks.size() );
    }
}

//-----------------------------------------------------------------------------
std::vector< CallstackEvent > EventBuffer::GetCallstackEvents( long long a_TimeBegin
                                                             , long long a_TimeEnd
                                                             , ThreadID a_ThreadId /*=-1*/)
{
    vector< CallstackEvent > callstackEvents;
    for( auto & pair : m_CallstackEvents )
    {
        ThreadID threadID = pair.first;
        std::map< long long, CallstackEvent > & callstacks = pair.second;

        if( a_ThreadId == -1 || threadID == a_ThreadId )
        {
            for( auto it = callstacks.lower_bound( a_TimeBegin ); it != callstacks.end(); ++it )
            {
                long long time = it->first;
                if( time < a_TimeEnd )
                {
                    callstackEvents.push_back( it->second );
                }
            }
        }
    }

    return callstackEvents;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( EventBuffer, 0 )
{
    ORBIT_NVP_VAL( 0, m_CallstackEvents );

    long long maxTime = m_MaxTime;
    ORBIT_NVP_VAL( 0, maxTime );
    m_MaxTime = maxTime;

    long long minTime = m_MinTime;
    ORBIT_NVP_VAL( 0, minTime );
    m_MinTime = minTime;
}

//-----------------------------------------------------------------------------
ORBIT_SERIALIZE( CallstackEvent, 0 )
{
    ORBIT_NVP_VAL( 0, m_Time );
    ORBIT_NVP_VAL( 0, m_Id );
    ORBIT_NVP_VAL( 0, m_TID );
}