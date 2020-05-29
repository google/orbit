//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/modern-cpp-threading/blob/master/LICENSE
//---------------------------------------------------------

#ifndef __MCPPT_AUTO_RESET_EVENT_COND_VAR_H__
#define __MCPPT_AUTO_RESET_EVENT_COND_VAR_H__

#include <cassert>
#include <mutex>
#include <condition_variable>


//---------------------------------------------------------
// AutoResetEventCondVar
//---------------------------------------------------------
class AutoResetEventCondVar
{
private:
    // m_status == 1: Event object is signaled.
    // m_status == 0: Event object is reset and no threads are waiting.
    // m_status == -N: Event object is reset and N threads are waiting.
    std::mutex m_mutex;
    int m_status;
    std::condition_variable m_condition;

public:
    AutoResetEventCondVar(int initialStatus = 0) : m_status(initialStatus)
    {
        assert(initialStatus >= 0 && initialStatus <= 1);
    }

    void signal()
    {
	    // Increment m_status atomically via critical section.
	    std::lock_guard<std::mutex> lock(m_mutex);
	    int oldStatus = m_status;
	    if (oldStatus == 1)
		    return;		// Event object is already signaled.
	    m_status++;
	    if (oldStatus < 0)
		    m_condition.notify_one();	// Release one waiting thread.
    }

    void wait()
    {
	    std::unique_lock<std::mutex> lock(m_mutex);
        int oldStatus = m_status;
        m_status--;
        assert(oldStatus <= 1);
        if (oldStatus < 1)
        {
            m_condition.wait(lock);
        }
    }
};


#endif // __MCPPT_AUTO_RESET_EVENT_COND_VAR_H__
