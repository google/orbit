//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <thread>
#include <unordered_map>
#include <functional>

class Module;

//-----------------------------------------------------------------------------
namespace LinuxUtils
{
    std::string ExecuteCommand( const char* a_Cmd );
    std::vector<std::string> ListModules( uint32_t a_PID );
    void ListModules( uint32_t a_PID, std::map< DWORD64, std::shared_ptr<Module> > & o_ModuleMap );
    uint64_t GetMicros( std::string a_TimeStamp );
    int Instructions();
    std::unordered_map<uint32_t, float> GetCpuUtilization();
    bool Is64Bit(uint32_t a_PID);
}

//-----------------------------------------------------------------------------
class LinuxPerf
{
public:
    LinuxPerf(uint32_t a_PID, uint32_t a_Freq = 1000);
    void Start();
    void Stop();
    bool IsRunning() const { return m_IsRunning; }
    void LoadPerfData( const std::string& a_FileName );

private:
    std::shared_ptr<std::thread> m_Thread;
    bool m_IsRunning = false;
    uint32_t m_PID = 0;
    uint32_t m_ForkedPID = 0;
    uint32_t m_Frequency = 1000;
    std::string m_OutputFile;
    std::string m_ReportFile;
};

//-----------------------------------------------------------------------------
class BpfTrace
{
public:
    typedef std::function<void(const std::string& a_Data)> Callback;

    BpfTrace(Callback a_Callback);
    void Start();
    void Stop();
    bool IsRunning() const { return !m_ExitRequested; }

private:
    std::shared_ptr<std::thread> m_Thread;
    bool m_ExitRequested = true;
    uint32_t m_PID = 0;
    Callback m_Callback;
};

//-----------------------------------------------------------------------------
struct LinuxSymbol
{
    std::string m_Module;
    std::string m_Name;
    std::string m_File;
    uint32_t    m_Line = 0;
};