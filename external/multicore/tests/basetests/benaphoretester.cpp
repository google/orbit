//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <vector>
#include <mutex>
#include "benaphore.h"


//---------------------------------------------------------
// BenaphoreTester
//---------------------------------------------------------
class BenaphoreTester
{
private:
    int m_iterationCount;
    NonRecursiveBenaphore m_mutex;
    int m_value;

public:
    BenaphoreTester() : m_iterationCount(0), m_value(0) {}

    void threadFunc(int threadNum)
    {
        for (int i = 0; i < m_iterationCount; i++)
        {
            m_mutex.lock();
         //   std::lock_guard<NonRecursiveBenaphore> lock(m_mutex);
            m_value++;
            m_mutex.unlock();
        }
    }

    bool test(int threadCount, int iterationCount)
    {
        m_iterationCount = iterationCount;
        m_value = 0;

        std::vector<std::thread> threads;
        for (int i = 0; i < threadCount; i++)
            threads.emplace_back(&BenaphoreTester::threadFunc, this, i);
        for (std::thread& t : threads)
            t.join();

        return (m_value == threadCount * iterationCount);
    }
};

bool testBenaphore()
{
    BenaphoreTester tester;
    return tester.test(4, 400000);
}
