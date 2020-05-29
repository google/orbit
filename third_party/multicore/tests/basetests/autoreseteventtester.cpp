//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <memory>
#include <atomic>
#include <random>
#include <string>
#include <thread>
#include "autoresetevent.h"


//---------------------------------------------------------
// AutoResetEventTester
//---------------------------------------------------------
class AutoResetEventTester
{
private:
    std::unique_ptr<AutoResetEvent[]> m_events;
    std::atomic<int> m_counter;
    int m_threadCount;
    int m_iterationCount;
    std::atomic<bool> m_success;

public:
    AutoResetEventTester()
    : m_counter(0)
    , m_threadCount(0)
    , m_success(0)
    {}

    void kickThreads(int exceptThread)
    {
        for (int i = 0; i < m_threadCount; i++)
        {
            if (i != exceptThread)
                m_events[i].signal();
        }
    }

    void threadFunc(int threadNum)
    {
        std::random_device rd;
        std::mt19937 randomEngine(rd());
        bool isKicker = (threadNum == 0);

        for (int i = 0; i < m_iterationCount; i++)
        {
            if (isKicker)
            {
                m_counter.store(m_threadCount, std::memory_order_relaxed);
                kickThreads(threadNum);
            }
            else
            {
                m_events[threadNum].wait();
            }

            // Decrement shared counter
            int previous = m_counter.fetch_sub(1, std::memory_order_relaxed);
            if (previous < 1)
                m_success.store(false, std::memory_order_relaxed);

            // Last one to decrement becomes the kicker next time
            isKicker = (previous == 1);

            // Do a random amount of work in the range [0, 10) units, biased towards low numbers.
            float f = std::uniform_real_distribution<float>(0.f, 1.f)(randomEngine);
            int workUnits = (int) (f * f * 10);
            for (int j = 1; j < workUnits; j++)
                randomEngine();       // Do one work unit
        }
    }

    bool test(int threadCount, int iterationCount)
    {
        m_events = std::unique_ptr<AutoResetEvent[]>(new AutoResetEvent[threadCount]);
        m_counter.store(0, std::memory_order_relaxed);
        m_threadCount = threadCount;
        m_iterationCount = iterationCount;
        m_success.store(true, std::memory_order_relaxed);

        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount; i++)
            threads.emplace_back(&AutoResetEventTester::threadFunc, this, i);
        for (std::thread& t : threads)
            t.join();

        return m_success.load(std::memory_order_relaxed);
    }
};

bool testAutoResetEvent()
{
    AutoResetEventTester tester;
    return tester.test(4, 1000000);
}
