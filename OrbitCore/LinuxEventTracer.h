//-----------------------------------
// Copyright Pierric Gimmig 2013-2019
//-----------------------------------
#pragma once

#include <thread>

//-----------------------------------------------------------------------------
class LinuxEventTracer
{
public:
    static const uint32_t ROUND_ROBIN_BATCH_SIZE = 5;
    typedef std::function<void(const std::string& a_Data)> Callback;
    void Start();
    void Stop();
    bool IsRunning() const { return !m_ExitRequested; }

protected:
    void CommandCallback(std::string a_Line);

private:
    bool m_ExitRequested = true;
    std::shared_ptr<std::thread> m_Thread;
    Callback m_Callback;

    static void Run(bool* a_ExitRequested);
};