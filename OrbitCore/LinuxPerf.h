//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "BaseTypes.h"
#include "SerializationMacros.h"
#include "Serialization.h"
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
    bool IsRunning() const { return m_IsRunning; }
    void LoadPerfData( const std::string& a_FileName );
    void LoadPerfData( std::istream& a_Stream );

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
struct LinuxSymbol
{
    std::string m_Module;
    std::string m_Name;
    std::string m_File;
    uint32_t    m_Line = 0;
    uint64_t    m_Address = 0;

    ORBIT_SERIALIZABLE;
};

ORBIT_SERIALIZE( LinuxSymbol, 0 )
{
    ORBIT_NVP_VAL( 0, m_Module );
    ORBIT_NVP_VAL( 0, m_Name );
    ORBIT_NVP_VAL( 0, m_File );
    ORBIT_NVP_VAL( 0, m_Line);
    ORBIT_NVP_VAL( 0, m_Address );
}