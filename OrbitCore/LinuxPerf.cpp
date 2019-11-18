//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include <iostream>
#include <fstream>

#include "LinuxUtils.h"
#include "Utils.h"
#include "PrintVar.h"
#include "Capture.h"
#include "Callstack.h"
#include "CoreApp.h"
#include "SamplingProfiler.h"
#include "EventBuffer.h"
#include "Capture.h"
#include "ScopeTimer.h"
#include "OrbitProcess.h"
#include "OrbitModule.h"
#include "ConnectionManager.h"
#include "TcpServer.h"
#include "EventBuffer.h"
#include "EventTracer.h"

#if __linux__
#include <unistd.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/types.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <iomanip>
#include <signal.h>
#include <sys/wait.h>
#include <sstream>
#include <string>
#include <cstdlib>
#include <memory>
#include <cxxabi.h>
#endif

//-----------------------------------------------------------------------------
LinuxPerf::LinuxPerf( uint32_t a_PID, uint32_t a_Freq )
        : m_PID(a_PID)
        , m_Frequency(a_Freq)
{
    m_Callback = [this](const std::string& a_Buffer)
    {
        HandleLine( a_Buffer );
    };
    m_PerfCommand = Format("perf record -k monotonic -F %u -p %u -g --no-buffering -o - | perf script -i -", m_Frequency, m_PID);
}

//-----------------------------------------------------------------------------
void LinuxPerf::Start()
{
    PRINT_FUNC;
#if __linux__
    m_ExitRequested = false;

    m_PerfData.Clear();

    m_Thread = std::make_shared<std::thread>
        ( &LinuxUtils::StreamCommandOutput
        , m_PerfCommand.c_str()
        , m_Callback
        , &m_ExitRequested
        );
    
    m_Thread->detach();
#endif
}

//-----------------------------------------------------------------------------
void LinuxPerf::Stop()
{
    PRINT_FUNC;
#if __linux__
    m_ExitRequested = true;
#endif
}

//-----------------------------------------------------------------------------
bool ParseStackLine( const std::string& a_Line, uint64_t& o_Address, std::string& o_Name, std::string& o_Module )
{
    // Module
    std::size_t moduleBegin = a_Line.find_last_of("(");
    if( moduleBegin == std::string::npos )
        return false;
    o_Module = RTrim(Replace(a_Line.substr(moduleBegin+1), ")", ""));

    // Function name
    std::string line = LTrim(a_Line.substr(0, moduleBegin));
    std::size_t nameBegin = line.find_first_of(" ");
    if( nameBegin == std::string::npos )
        return false;
    o_Name = line.substr(nameBegin);

    // Address
    std::string address = line.substr(0, nameBegin);
    o_Address = std::stoull(address, nullptr, 16);

    return true;
}

void LinuxPerf::HandleLine( const std::string& a_Line )
{
    bool isEmptyLine = (a_Line.empty() || a_Line == "\n");
    bool isHeader = !isEmptyLine && !StartsWith(a_Line, "\t");
    bool isStackLine = !isHeader && !isEmptyLine;
    bool isEndBlock = !isStackLine && isEmptyLine && !m_PerfData.m_header.empty();

    if(isHeader)
    {
        m_PerfData.m_header = a_Line;
        auto tokens = Tokenize(a_Line);
        m_PerfData.m_time = tokens.size() > 2 ? GetMicros(tokens[2])*1000 : 0;
        m_PerfData.m_tid  = tokens.size() > 1 ? atoi(tokens[1].c_str()) : 0;
    }
    else if(isStackLine)
    {
        std::string module;
        std::string function;
        uint64_t    address;

        if( !ParseStackLine( a_Line, address, function, module ) )
        {
            PRINT_VAR("ParseStackLine error");
            PRINT_VAR(a_Line);
            return;
        }

        std::wstring moduleName = ToLower(Path::GetFileName(s2ws(module)));
        std::shared_ptr<Module> moduleFromName = Capture::GTargetProcess->GetModuleFromName( ws2s(moduleName) );

        if( moduleFromName )
        {
            uint64_t new_address = moduleFromName->ValidateAddress(address);
            address = new_address;
        }

        m_PerfData.m_CS.m_Data.push_back(address);
        if( Capture::GTargetProcess && !Capture::GTargetProcess->HasSymbol(address))
        {
            GCoreApp->AddSymbol(address, module, function);
        }
    }
    else if(isEndBlock)
    {
        CallStack& CS = m_PerfData.m_CS;
        if( CS.m_Data.size() )
        {
            CS.m_Depth = (uint32_t)CS.m_Data.size();
            CS.m_ThreadId = m_PerfData.m_tid;
            CallstackID hash = CS.Hash();

            GCoreApp->ProcessSamplingCallStack(m_PerfData);
            ++m_PerfData.m_numCallstacks;

            Capture::GSamplingProfiler->AddCallStack( CS );
            GEventTracer.GetEventBuffer().AddCallstackEvent( m_PerfData.m_time, hash, m_PerfData.m_tid );
        }

        m_PerfData.Clear();
    }
}

//-----------------------------------------------------------------------------
void LinuxPerf::LoadPerfData( std::istream& a_Stream )
{
    m_PerfData.Clear();

    for (std::string line; std::getline(a_Stream, line); )
    {
        HandleLine(line);
    }

    PRINT_VAR(m_PerfData.m_numCallstacks);
    PRINT_FUNC;
}
