#include "OrbitTest.h"

#include <iostream>
#include <thread>
#include <chrono>

static const uint32_t NUM_THREADS = 10;
static const uint32_t NUM_RECURSIVE_CALLS = 10;

#if __linux__
#define NO_INLINE __attribute__ ((noinline))
#else
#define NO_INLINE __declspec(noinline)
#endif

//-----------------------------------------------------------------------------
OrbitTest::OrbitTest()
{
}

//-----------------------------------------------------------------------------
OrbitTest::~OrbitTest()
{
    m_ExitRequested = true;

    for (uint32_t i = 0; i < NUM_THREADS; ++i)
    {
        m_Threads[i]->join();
    }
}

//-----------------------------------------------------------------------------
void OrbitTest::Start()
{
    for (uint32_t i = 0; i < NUM_THREADS; ++i)
    {
        std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(&OrbitTest::ThreadLoop, this);
        m_Threads.push_back(thread);
    }
}

//-----------------------------------------------------------------------------
void OrbitTest::ThreadLoop()
{
    while (!m_ExitRequested)
    {
        TestFunc();
    }
}
//-----------------------------------------------------------------------------
void NO_INLINE OrbitTest::TestFunc(uint32_t a_Depth)
{
    if (a_Depth == NUM_RECURSIVE_CALLS)
        return;

    TestFunc(a_Depth + 1);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}