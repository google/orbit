//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------

//-----------------------------------
// Author: Florian Kuebler
//-----------------------------------
#pragma once

#include <thread>

//-----------------------------------------------------------------------------
class LinuxSampling
{
public:
    LinuxSampling(uint32_t a_PID, uint32_t a_Freq = 1000);
    void Start();
    void Stop();
    bool IsRunning() const { return !m_ExitRequested; }
    void RunPerfEventOpen( bool* a_ExitRequested );

private:
    uint32_t m_PID = 0;
    uint32_t m_ForkedPID = 0;
    uint32_t m_Frequency = 1000;

    std::shared_ptr<std::thread> m_Thread;
    bool m_ExitRequested = true;
};