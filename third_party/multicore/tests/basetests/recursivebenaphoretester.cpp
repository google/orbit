//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <vector>
#include <mutex>
#include <string>
#include <random>
#include "benaphore.h"


//---------------------------------------------------------
// RecursiveBenaphoreTester
//---------------------------------------------------------
class RecursiveBenaphoreTester
{
private:
    struct ThreadStats
    {
        int iterations;
        int workUnitsComplete;
        int amountIncremented;

        ThreadStats()
        {
            iterations = 0;
            workUnitsComplete = 0;
            amountIncremented = 0;
        }

        ThreadStats& operator+=(const ThreadStats &other)
        {
            iterations += other.iterations;
            workUnitsComplete += other.workUnitsComplete;
            amountIncremented += other.amountIncremented;
            return *this;
        }
    };

    int m_iterationCount;
    RecursiveBenaphore m_recursiveMutex;
    int m_value;
    std::vector<ThreadStats> m_threadStats;

public:
    RecursiveBenaphoreTester() : m_iterationCount(0), m_value(0) {}

    void threadFunc(int threadNum)
    {
        std::random_device rd;
        std::mt19937 randomEngine(rd());
        ThreadStats localStats;
        int lockCount = 0;
        int lastValue = 0;

        for (int i = 0; i < m_iterationCount; i++)
        {
            localStats.iterations++;

            // Do a random amount of work.
            int workUnits = std::uniform_int_distribution<>(0, 3)(randomEngine);
            for (int j = 1; j < workUnits; j++)
                randomEngine();       // Do one work unit
            localStats.workUnitsComplete += workUnits;

            // Consistency check.
            if (lockCount > 0)
            {
                assert(m_value == lastValue);
            }

            // Decide what the new lock count should be in the range [0, 4), biased towards low numbers.
            float f = std::uniform_real_distribution<float>(0.f, 1.f)(randomEngine);
            int desiredLockCount = (int) (f * f * 4);

            // Perform unlocks, if any.
            while (lockCount > desiredLockCount)
            {
                m_recursiveMutex.unlock();
                lockCount--;
            }

            // Perform locks, if any.
            bool useTryLock = (std::uniform_int_distribution<>(0, 1)(randomEngine) == 0);
            while (lockCount < desiredLockCount)
            {
                if (useTryLock)
                {
                    if (!m_recursiveMutex.tryLock())
                        break;
                }
                else
                {
                    m_recursiveMutex.lock();
                }
                lockCount++;
            }

            // If locked, increment counter.
            if (lockCount > 0)
            {
                assert((m_value - lastValue) >= 0);
                m_value += threadNum + 1;
                lastValue = m_value;
                localStats.amountIncremented += threadNum + 1;
            }
        }

        // Release Lock if still holding it.
        while (lockCount > 0)
        {
            m_recursiveMutex.unlock();
            lockCount--;
        }

        // Copy statistics.
        m_threadStats[threadNum] = localStats;
    }

    bool test(int threadCount, int iterationCount)
    {
        m_iterationCount = iterationCount;
        m_value = 0;
        m_threadStats.resize(threadCount);

        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount; i++)
            threads.emplace_back(&RecursiveBenaphoreTester::threadFunc, this, i);
        for (std::thread& t : threads)
            t.join();

        ThreadStats totalStats;
        for (const ThreadStats& s : m_threadStats)
            totalStats += s;
        return (m_value == totalStats.amountIncremented);
    }
};

bool testRecursiveBenaphore()
{
    RecursiveBenaphoreTester tester;
    return tester.test(4, 100000);
}
