//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <mutex>
#include <condition_variable>
#include <thread>
#include <concurrentqueue.h>
#include <autoresetevent.h>
#include "ScopeTimer.h"

#define OQPI_USE_DEFAULT
#include "../external/oqpi/include/oqpi.hpp"

typedef std::recursive_mutex                    Mutex;
typedef std::lock_guard<std::recursive_mutex>   ScopeLock;
typedef std::unique_lock<std::recursive_mutex>  UniqueLock;
typedef std::condition_variable                 ConditionVariable;
typedef AutoResetEvent                          AutoResetEvent;

template<typename T>
using LockFreeQueue = moodycamel::ConcurrentQueue<T>;
using oqpi_tk       = oqpi::default_helpers;

//-----------------------------------------------------------------------------
class ConditionalLock
{
public:
    ConditionalLock( Mutex & a_Mutex ) : m_Mutex(&a_Mutex), m_Locked(false) {}
    ~ConditionalLock() { if( m_Locked ) m_Mutex->unlock(); }
    void Lock() 
    { 
        {
            m_Locked=true;
            m_Mutex->lock();
        }
        
        m_Timer.Start("Critical Section");
    }

private:
    ConditionalScopeTimer m_Timer;
    Mutex*                m_Mutex;
    bool                  m_Locked;
};

const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

//-----------------------------------------------------------------------------
inline void SetThreadName( DWORD dwThreadID, char* threadName )
{
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
    {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
    }
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
    }
}

