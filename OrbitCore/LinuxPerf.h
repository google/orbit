//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include "LinuxPerfData.h"
#include <string>
#include <memory>
#include <vector>
#include <thread>

//-----------------------------------------------------------------------------
class LinuxPerf
{
public:
    LinuxPerf(uint32_t a_PID, uint32_t a_Freq = 1000);
    void Start();
    void Stop();
    bool IsRunning() const { return !m_ExitRequested; }
    void LoadPerfData( std::istream& a_Stream );
    void HandleLine( const std::string& a_Line );

private:
    uint32_t m_PID = 0;
    uint32_t m_ForkedPID = 0;
    uint32_t m_Frequency = 1000;

    std::shared_ptr<std::thread> m_Thread;
    bool m_ExitRequested = true;

    std::function<void(const std::string& a_Data)> m_Callback;
    std::string m_PerfCommand;

    LinuxPerfData m_PerfData;
};

//-----------------------------------------------------------------------------
struct LinuxSymbol
{
    std::string m_Module;
    std::string m_Name;
    std::string m_File;
    uint32_t    m_Line = 0;
};