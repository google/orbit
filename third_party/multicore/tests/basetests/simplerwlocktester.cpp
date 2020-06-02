//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <vector>
#include <mutex>
#include <string>
#include <thread>
#include "rwlock.h"


//---------------------------------------------------------
// SimpleRWLockTester
// Writes less frequently and only protects a single int.
//---------------------------------------------------------
class SimpleRWLockTester
{
private:
    NonRecursiveRWLock m_rwLock;
    int m_sharedInt;
    int m_iterationCount;
    std::atomic<int> m_totalWrites;

public:
    SimpleRWLockTester()
    : m_iterationCount(0)
    , m_totalWrites(0)
    {}

    void threadFunc(int threadNum)
    {
        std::random_device rd;
        std::mt19937 randomEngine(rd());
        int writes = 0;
        volatile int accumulator = 0;   // Prevent compiler from eliminating this variable

        for (int i = 0; i < m_iterationCount; i++)
        {
            // Choose randomly whether to read or write.
            if (std::uniform_int_distribution<>(0, 30)(randomEngine) == 0)
            {
                WriteLockGuard<NonRecursiveRWLock> guard(m_rwLock);
                m_sharedInt++;
                writes++;
            }
            else
            {
                ReadLockGuard<NonRecursiveRWLock> guard(m_rwLock);
                accumulator += m_sharedInt;
            }
        }

        m_totalWrites.fetch_add(writes, std::memory_order_relaxed);
    }

    bool test(int threadCount, int iterationCount)
    {
        m_iterationCount = iterationCount;
        m_sharedInt = 0;
        m_totalWrites.store(0, std::memory_order_relaxed);

        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount; i++)
            threads.emplace_back(&SimpleRWLockTester::threadFunc, this, i);
        for (std::thread& t : threads)
            t.join();

        return (m_sharedInt == m_totalWrites.load(std::memory_order_relaxed));
    }
};

bool testRWLockSimple()
{
    SimpleRWLockTester tester;
    return tester.test(4, 2000000);
}
