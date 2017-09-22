//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <memory>
#include <random>
#include "inmemorylogger.h"


InMemoryLogger::InMemoryLogger()
    : m_head(new Page)
    , m_tail(m_head.get())
{
}

InMemoryLogger::Event* InMemoryLogger::allocateEventFromNewPage()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    // Double-checked locking:
    // Check again whether the current page is full. Another thread may have called
    // allocateEventFromNewPage and created a new page by the time we get take the lock.
    Page* oldTail = m_tail.load(std::memory_order_relaxed);
    if (oldTail->index.load(std::memory_order_relaxed) < EVENTS_PER_PAGE)
    {
        int index = oldTail->index.fetch_add(1, std::memory_order_relaxed);
        // Yes! We got a slot on this page.
        if (index < EVENTS_PER_PAGE)
            return &oldTail->events[index];
    }

    // OK, we're definitely out of space. It's up to us to allocate a new page.
    std::unique_ptr<Page> newTail(new Page);
    Page* page = newTail.get();
    // Reserve the first slot. Relaxed is fine because there will be a release/
    // consume relationship via m_tail before any other threads access the index.
    page->index.store(1, std::memory_order_relaxed);
    // A plain non-atomic move to oldTail->next is fine because there are no other writers here,
    // and nobody is supposed to read the logged contents until all logging is complete.
    oldTail->next = std::move(newTail);
    // m_tail must be written atomically because it is read concurrently from other threads.
    // We also use release/consume semantics so that its constructed contents are visible to other threads.
    // Again, very much like the double-checked locking pattern.
    m_tail.store(page, std::memory_order_release);

    // Return the reserved slot.
    return &page->events[0];
}
