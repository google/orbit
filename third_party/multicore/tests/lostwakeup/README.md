This project tests `AutoResetEvent` for lost wakeups.

The original implementation of `AutoResetEvent::signal()`, [as of commit 3347437](https://github.com/preshing/cpp11-on-multicore/tree/3347437911bfd3a45ddc07cb1cbb4a9d3e55583c), looked like this:

    void signal()
    {
        int oldStatus = m_status.load(std::memory_order_relaxed);
        for (;;)    // Increment m_status atomically via CAS loop.
        {
            assert(oldStatus <= 1);
            if (oldStatus == 1)
                return;     // Event object is already signaled.
            int newStatus = oldStatus + 1;
            if (m_status.compare_exchange_weak(oldStatus, newStatus, std::memory_order_release, std::memory_order_relaxed))
                break;
            // The compare-exchange failed, likely because another thread changed m_status.
            // oldStatus has been updated. Retry the CAS loop.
        }
        if (oldStatus < 0)
            m_sema.signal();    // Release one waiting thread.
    }


Commenter [Tobias Brüll correctly pointed out](http://preshing.com/20150316/semaphores-are-surprisingly-versatile) that this implementation was flawed, due to the early return in the middle of the CAS loop:

> Isn't there the possibility of a lost wakeup in AutoResetEvent? Imagine two threads T1, T2 pushing requests, one thread W popping requests, an object `are` of type `AutoResetEvent` and the following course of actions: 
> 
> 1.) T1 pushes an item, calls `are.signal()`, and runs to termination. We now have `are.m_status == 1`. 
> 
> 2.) T2 reorders the `m_status.load(std::memory_order_relaxed)` before `requests.push(r)` to obtain 1. 
> 
> 3.) W runs. First it calls `are.wait()` and sets `are.m_status = 0`. W then empties the `requests`-queue; processing only one item. In the subsequent call to `are.wait()` it blocks on the semaphore. 
> 
> 4.) T2 continues. The effects of `requests.push(r)` become visible. However, since oldStatus == 1 in are.signal(), nothing happens in T2's call to are.signal(). 
> 
> 5.) W continues to block on the semaphore, although there is work to do. 

Essentially, AutoResetEvent::signal() should always perform a release operation. That's the only way it can [_synchronize-with_](http://preshing.com/20130823/the-synchronizes-with-relation) the waiting thread. The bug exists because when the signaling thread takes that early return, it doesn't perform any release operation.

The fix is to eliminate that early return and let it effectively perform `compare_exchange_weak(1, 1, std::memory_order_release)`:

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

The test in this folder can both reproduce the original bug and validate the fix. Build and run it using the same steps as `tests/basetests`, as described in the [root README file](https://github.com/preshing/cpp11-on-multicore/blob/master/README.md).
