//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#ifndef __CPP11OM_IN_MEMORY_LOGGER_H__
#define __CPP11OM_IN_MEMORY_LOGGER_H__

#include <thread>
#include <mutex>
#include <memory>
#include <atomic>


//---------------------------------------------------------
// InMemberLogger
// Logs an unbounded number of generic events.
// Each event has a const char* message and a size_t param.
// log() is usually lock-free, except when it's time allocate a new Page.
// Iterator should only be used after logging is complete.
// Useful for post-mortem debugging and for validating tests, as DiningPhilosopherTester does.
//---------------------------------------------------------
class InMemoryLogger
{
public:
    struct Event
    {
        std::thread::id tid;
        const char* msg;
        size_t param;

        Event() : msg(nullptr), param(0) {}
    };

private:
    static const int EVENTS_PER_PAGE = 16384;

    struct Page
    {
        std::unique_ptr<Page> next;
        std::atomic<int> index;     // This can exceed EVENTS_PER_PAGE, but it's harmless. Just means page is full.
        Event events[EVENTS_PER_PAGE];

        Page() : index(0) {}
    };

    // Store events in a linked list of pages.
    // Mutex is only locked when it's time to allocate a new page.
    std::mutex m_mutex;
    std::unique_ptr<Page> m_head;
    std::atomic<Page*> m_tail;

    Event* allocateEventFromNewPage();

public:
    InMemoryLogger();

    void log(const char* msg, size_t param = 0)
    {
        std::atomic_signal_fence(std::memory_order_seq_cst);    // Compiler barrier
        // On weak CPUs and current C++ compilers, memory_order_consume costs the same as acquire. :(
        // (If you don't like that, you can probably demote this load to relaxed and get away with it.
        // Technically, you'd be violating the spec, but in practice it will likely work. Just
        // inspect the assembly and make sure there is a data dependency between m_tail.load and
        // both subsequent uses of page, and you're golden. The only way I can imagine the dependency
        // chain being broken is if the compiler knows the addresses that will be allocated
        // in allocateEventFromNewPage at runtime, which is a huuuuuuuuuge leap of the imagination.)
        // http://preshing.com/20140709/the-purpose-of-memory_order_consume-in-cpp11
        Page* page = m_tail.load(std::memory_order_consume);
        Event* evt;
        int index = page->index.fetch_add(1, std::memory_order_relaxed);
        if (index < EVENTS_PER_PAGE)
            evt = &page->events[index];
        else
            evt = allocateEventFromNewPage();   // Double-checked locking is performed inside here.
        evt->tid = std::this_thread::get_id();
        evt->msg = msg;
        evt->param = param;
        std::atomic_signal_fence(std::memory_order_seq_cst);    // Compiler barrier
    }

    // Iterators are meant to be used only after all logging is complete.
    friend class Iterator;
    class Iterator
    {
    private:
        Page* m_page;
        int m_index;

    public:
        Iterator(Page* p, int i) : m_page(p), m_index(i) {}

        Iterator& operator++()
        {
            m_index++;
            if (m_index >= EVENTS_PER_PAGE)
            {
                Page* next = m_page->next.get();
                if (next)
                {
                    m_page = next;
                    m_index = 0;
                }
                else
                {
                    m_index = m_page->index;
                }
            }
            return *this;
        }

        bool operator!=(const Iterator& other) const
        {
            return (m_page != other.m_page) || (m_index != other.m_index);
        }

        const Event& operator*() const
        {
            return m_page->events[m_index];
        }
    };

    Iterator begin()
    {
        return Iterator(m_head.get(), 0);
    }

    Iterator end()
    {
        Page* tail = m_tail.load(std::memory_order_relaxed);
        return Iterator(tail, tail->index);
    }
};


#endif // __CPP11OM_IN_MEMORY_LOGGER_H__
