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
// RWLockTester
//---------------------------------------------------------
class RWLockTester
{
private:
    static const int SHARED_ARRAY_LENGTH = 8;
    int m_shared[SHARED_ARRAY_LENGTH];
    NonRecursiveRWLock m_rwLock;
    int m_iterationCount;
    std::atomic<bool> m_success;

public:
    RWLockTester()
    : m_iterationCount(0)
    , m_success(false)
    {}

    void threadFunc(int threadNum)
    {
        std::random_device rd;
        std::mt19937 randomEngine(rd());

        for (int i = 0; i < m_iterationCount; i++)
        {
            // Choose randomly whether to read or write.
            if (std::uniform_int_distribution<>(0, 3)(randomEngine) == 0)
            {
                // Write an incrementing sequence of numbers (backwards).
                int value = std::uniform_int_distribution<>()(randomEngine);
                WriteLockGuard<NonRecursiveRWLock> guard(m_rwLock);
                for (int j = SHARED_ARRAY_LENGTH - 1; j >= 0; j--)
                {
                    m_shared[j] = value--;
                }
            }
            else
            {
                // Check that the sequence of numbers is incrementing.
                bool ok = true;
                {
                    ReadLockGuard<NonRecursiveRWLock> guard(m_rwLock);
                    int value = m_shared[0];
                    for (int j = 1; j < SHARED_ARRAY_LENGTH; j++)
                    {
                        ok = ok && (++value == m_shared[j]);
                    }
                }
                if (!ok)
                {
                    m_success.store(false, std::memory_order_relaxed);
                }
            }
        }
    }

    bool test(int threadCount, int iterationCount)
    {
        m_iterationCount = iterationCount;
        for (int j = 0; j < SHARED_ARRAY_LENGTH; j++)
            m_shared[j] = j;
        m_success.store(true, std::memory_order_relaxed);

        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount; i++)
            threads.emplace_back(&RWLockTester::threadFunc, this, i);
        for (std::thread& t : threads)
            t.join();

        return m_success.load(std::memory_order_relaxed);
    }
};

bool testRWLock()
{
    RWLockTester tester;
    return tester.test(4, 1000000);
}
