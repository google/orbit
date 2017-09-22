//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitThread.h"
#include "Utils.h"

//-----------------------------------------------------------------------------
void Thread::UpdateUsage()
{
    m_Usage.Add( GetUsage() );
}

//-----------------------------------------------------------------------------
float Thread::GetUsage()
{
    if( m_Handle )
    {
        FILETIME CreationTime;
        FILETIME ExitTime;
        FILETIME KernelTime;
        FILETIME UserTime;

        if( GetThreadTimes(m_Handle, &CreationTime, &ExitTime, &KernelTime, &UserTime) )
        {
            double elapsedMillis = m_UpdateThreadTimer.QueryMillis();
            m_UpdateThreadTimer.Start();

            LONGLONG kernMs = FileTimeDiffInMillis(m_LastKernTime, KernelTime);
            LONGLONG userMs = FileTimeDiffInMillis(m_LastUserTime, UserTime);

            m_LastKernTime = KernelTime;
            m_LastUserTime = UserTime;

            double threadUsage = 0.f;
            if( m_Init )
            {
                threadUsage = (100.0 * double(kernMs + userMs) / elapsedMillis);
            }

            m_Init = true;
            return (float)threadUsage;
        }
    }

    return -1.f;
}
