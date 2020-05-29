//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#ifndef __CPP11OM_AUTO_RESET_EVENT_H__
#define __CPP11OM_AUTO_RESET_EVENT_H__

#include <cassert>
#include <thread>
#include "sema.h"


//---------------------------------------------------------
// AutoResetEvent
//---------------------------------------------------------
class AutoResetEvent
{
private:
    // m_status == 1: Event object is signaled.
    // m_status == 0: Event object is reset and no threads are waiting.
    // m_status == -N: Event object is reset and N threads are waiting.
    std::atomic<int> m_status;
    DefaultSemaphoreType m_sema;

public:
    AutoResetEvent(int initialStatus = 0) : m_status(initialStatus)
    {
        assert(initialStatus >= 0 && initialStatus <= 1);
    }

    void signal()
    {
        int oldStatus = m_status.load(std::memory_order_relaxed);
        for (;;)    // Increment m_status atomically via CAS loop.
        {
            assert(oldStatus <= 1);
            int newStatus = oldStatus < 1 ? oldStatus + 1 : 1;
            if (m_status.compare_exchange_weak(oldStatus, newStatus, std::memory_order_release, std::memory_order_relaxed))
                break;
            // The compare-exchange failed, likely because another thread changed m_status.
            // oldStatus has been updated. Retry the CAS loop.
        }
        if (oldStatus < 0)
            m_sema.signal();    // Release one waiting thread.
    }

    void wait()
    {
        int oldStatus = m_status.fetch_sub(1, std::memory_order_acquire);
        assert(oldStatus <= 1);
        if (oldStatus < 1)
        {
            m_sema.wait();
        }
    }
};


#endif // __CPP11OM_AUTO_RESET_EVENT_H__
