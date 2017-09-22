//---------------------------------------------------------
// For conditions of distribution and use, see
// https://github.com/preshing/cpp11-on-multicore/blob/master/LICENSE
//---------------------------------------------------------

#include <iostream>
#include <string>
#include <ctime>
#include <thread>
#include <random>
#include <chrono>
#include <memory>
#include <atomic>
#include "autoresetevent.h"


std::string makeTimeString(const std::chrono::time_point<std::chrono::system_clock>& point)
{
    std::time_t time = std::chrono::system_clock::to_time_t(point);
    char str[256];
    if (strftime(str, sizeof(str), "%c", std::localtime(&time)))
        return str;
    else
        return "???";
}


//---------------------------------------------------------
// LostWakeupTester
//---------------------------------------------------------
class LostWakeupTester
{
private:
    struct Wrapper
    {
        std::atomic<int> value;
        Wrapper() : value(0) {}
    };

    struct ThreadData
    {
        std::atomic<bool> canStart;
        std::atomic<bool> finished;
        ThreadData() : canStart(false), finished(false) {}
    };

    AutoResetEvent m_event;
    static const int kWorkAreaSize = 10000000;
    std::unique_ptr<Wrapper[]> m_workArea;
    int m_workIndex;
    ThreadData m_threadData[3];

public:
    LostWakeupTester()
    : m_workArea(new Wrapper[kWorkAreaSize])
    , m_workIndex(0)
    {
    }

    void threadFunc(int threadNum)
    {
        ThreadData& td = m_threadData[threadNum];
        for (;;)
        {
            // Spin-wait for kick signal
            while (!td.canStart)
                std::atomic_signal_fence(std::memory_order_seq_cst);
            td.canStart = false;

            // Do this thread's job
            int workIndex = m_workIndex;
            if (threadNum == 0)
            {
                // Thread #0 "consumes work items" until signaled to stop
                for (;;)
                {
                    m_event.wait();
                    int previous = m_workArea[workIndex].value.exchange(0, std::memory_order_relaxed);
                    if (previous == -1)
                        break;
                }
            }
            else
            {
                // Thread #1 and #2 each "publish a work item"
                m_workArea[workIndex].value.store(1, std::memory_order_relaxed);
                m_event.signal();
            }

            // Notify main thread that we've finished
            td.finished = true;
        }
    }

    bool test()
    {
        std::random_device rd;
        std::mt19937 randomEngine(rd());
        auto start = std::chrono::system_clock::now();
        std::cout << "[" << makeTimeString(start) << "] start \n";
        uint64_t failures = 0;
        uint64_t trials = 0;
        static const double kLogInterval = 1.0;
        static const double kTimeout = 0.25;
        double nextLogTime = kLogInterval;

        // Spawn threads
        std::thread t0(&LostWakeupTester::threadFunc, this, 0);
        std::thread t1(&LostWakeupTester::threadFunc, this, 1);
        std::thread t2(&LostWakeupTester::threadFunc, this, 2);

        for (;;)
        {
            trials++;

            // Initialize experiment
            m_workIndex = std::uniform_int_distribution<>(0, kWorkAreaSize - 1)(randomEngine);
            m_workArea[m_workIndex].value = 0;
            for (ThreadData& td : m_threadData)
                td.finished = false;

            // Kick threads
            for (ThreadData& td : m_threadData)
                td.canStart = true;

            // Wait for t1 + t2
            while (!m_threadData[1].finished)
                std::atomic_signal_fence(std::memory_order_seq_cst);
            while (!m_threadData[2].finished)
                std::atomic_signal_fence(std::memory_order_seq_cst);

            // t0 should have consumed all "work items" within a reasonable time frame
            auto startOfTimeout = std::chrono::high_resolution_clock::now();
            while (m_workArea[m_workIndex].value != 0)
            {
                auto elapsed = std::chrono::high_resolution_clock::now() - startOfTimeout;
                if (std::chrono::duration_cast<std::chrono::duration<double>>(elapsed).count() >= kTimeout)
                {
                    failures++;
                    break;
                }
            }

            // Stop t0
            m_workArea[m_workIndex].value.store(-1, std::memory_order_relaxed);
            while (!m_threadData[0].finished)
            {
                m_event.signal();
                std::atomic_signal_fence(std::memory_order_seq_cst);
            }

            // Log the rate
            auto now = std::chrono::system_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::duration<double>>(now - start).count();
            if (elapsed >= nextLogTime)
            {
                std::cout << "[" << makeTimeString(std::chrono::system_clock::now()) << "] "
                    << failures << " failures out of " << trials << ", "
                    << (trials / elapsed) << " trials/sec\n";
                nextLogTime = elapsed + kLogInterval;
            }
        }
    }
};

int main()
{
    LostWakeupTester tester;
    return tester.test() ? 0 : 1;
}
