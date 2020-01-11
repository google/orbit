#pragma once
#include <vector>
#include <thread>
#include <memory>

class OrbitTest
{
public:
    OrbitTest();
    ~OrbitTest();

    void Start();
    void Stop();
    void Loop();
    void TestFunc(uint32_t a_Depth = 0);

private:
    bool m_ExitRequested = false;
    std::vector<std::shared_ptr<std::thread>> m_Threads;
};
