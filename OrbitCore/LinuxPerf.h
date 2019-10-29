//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
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
    void AttachProbes();
    void DetachProbes();
    bool IsRunning() const { return m_IsRunning; }
    void LoadPerfData( const std::string& a_FileName );
    void LoadPerfData( std::istream& a_Stream );

private:
    std::shared_ptr<std::thread> m_Thread;
    bool m_IsRunning = false;
    uint32_t m_PID = 0;
    uint32_t m_ForkedPIDSampling = 0;
    uint32_t m_ForkedPIDUProbes = 0;
    uint32_t m_Frequency = 1000;
    std::string m_OutputFileSampling;
    std::string m_OutputFileUProbes;
    std::string m_ReportFileSampling;
    std::string m_ReportFileUProbes;
};

//-----------------------------------------------------------------------------
struct LinuxSymbol
{
    std::string m_Module;
    std::string m_Name;
    std::string m_File;
    uint32_t    m_Line = 0;
};