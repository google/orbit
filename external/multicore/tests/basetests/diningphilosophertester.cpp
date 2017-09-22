//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <memory>
#include <random>
#include <string>
#include <thread>
#include <cstring>
#include "diningphilosophers.h"
#include "inmemorylogger.h"


//---------------------------------------------------------
// DiningPhilosopherTester
//---------------------------------------------------------
class DiningPhilosopherTester
{
private:
    InMemoryLogger m_logger;
    std::unique_ptr<DefaultDiningPhilosophersType> m_philosophers;
    int m_iterationCount;

public:
    DiningPhilosopherTester() : m_iterationCount(0) {}

    void threadFunc(int philoIndex)
    {
        std::random_device rd;
        std::mt19937 randomEngine(rd());

        for (int i = 0; i < m_iterationCount; i++)
        {
            // Do a random amount of work.
            int workUnits = std::uniform_int_distribution<int>(0, 100)(randomEngine);
            for (int j = 0; j < workUnits; j++)
                randomEngine();

            m_philosophers->beginEating(philoIndex);
            m_logger.log("eat", philoIndex);

            // Do a random amount of work.
            workUnits = std::uniform_int_distribution<int>(0, 5000)(randomEngine);
            for (int j = 0; j < workUnits; j++)
                randomEngine();

            m_logger.log("think", philoIndex);
            m_philosophers->endEating(philoIndex);
        }
    }

    bool test(int numPhilos, int iterationCount)
    {
        m_iterationCount = iterationCount;
        m_philosophers = std::unique_ptr<DefaultDiningPhilosophersType>(new DefaultDiningPhilosophersType(numPhilos));

        std::vector<std::thread> threads;
        for (int i = 0; i < numPhilos; i++)
            threads.emplace_back(&DiningPhilosopherTester::threadFunc, this, i);
        for (std::thread& t : threads)
            t.join();

        // Replay event log to make sure it's OK.
        std::vector<char> isEating(numPhilos);
        bool ok = true;
        for (const auto& evt : m_logger)
        {
            int philoIndex = (int) evt.param;
            if (std::strcmp(evt.msg, "eat") == 0)
            {
                if (isEating[philoIndex])
                    ok = false;
                if (isEating[DiningPhiloHelpers::left(philoIndex, numPhilos)] != 0)
                    ok = false;
                if (isEating[DiningPhiloHelpers::right(philoIndex, numPhilos)] != 0)
                    ok = false;
                isEating[philoIndex] = 1;
            }
            else
            {
                assert(std::strcmp(evt.msg, "think") == 0);
                if (!isEating[philoIndex])
                    ok = false;
                isEating[philoIndex] = 0;
            }
        }
        for (char s : isEating)
        {
            if (s)
                ok = false;
        }

        m_philosophers = nullptr;
        return ok;
    }
};

bool testDiningPhilosophers()
{
    DiningPhilosopherTester tester;
    return tester.test(5, 10000);
}

