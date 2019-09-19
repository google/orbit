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
#include "SamplingProfiler.h"
#include "EventBuffer.h"
#include "Capture.h"
#include "ScopeTimer.h"
#include "OrbitProcess.h"
#include "OrbitModule.h"
#include "ConnectionManager.h"
#include "TcpClient.h"
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
    m_OutputFile = "/tmp/perf.data";
    m_ReportFile = Replace(m_OutputFile, ".data", ".txt");
}

//-----------------------------------------------------------------------------
void LinuxPerf::Start()
{
#if __linux__
    m_IsRunning = true;

    std::string path = ws2s(Path::GetBasePath());

    pid_t PID = fork();
    if(PID == 0) {
        execl   ( "/usr/bin/perf"
                , "/usr/bin/perf"
                , "record"
                , "-k", "monotonic"
                , "-F", Format("%u", m_Frequency).c_str()
                , "-p", Format("%u", m_PID).c_str()
                , "-g"
                , "-o", m_OutputFile.c_str(), (const char*)nullptr);
        exit(1);
    }
    else
    {
        m_ForkedPID = PID;
        PRINT_VAR(m_ForkedPID);
    }
#endif
}

//-----------------------------------------------------------------------------
void LinuxPerf::Stop()
{
#if __linux__
    if( m_ForkedPID != 0 )
    {
        kill(m_ForkedPID, 15);
        int status = 0;
        waitpid(m_ForkedPID, &status, 0);
    }

    m_IsRunning = false;

    //m_Thread = std::make_shared<std::thread>([this]{
    std::string cmd = Format("perf script -i %s > %s", m_OutputFile.c_str(), m_ReportFile.c_str());
    PRINT_VAR(cmd);
    LinuxUtils::ExecuteCommand(cmd.c_str());

    if( ConnectionManager::Get().IsRemote() )
    {
        std::ifstream t(m_ReportFile);
        std::stringstream buffer;
        buffer << t.rdbuf();
        std::string perfData = buffer.str();
        GTcpServer->Send(Msg_RemotePerf, (void*)perfData.data(), perfData.size());
    }
    //else
    {
        LoadPerfData(m_ReportFile);
    }
    // });

    // m_Thread->detach();
#endif
}

//-----------------------------------------------------------------------------
bool ParseStackLine( const std::string& a_Line, uint64_t& o_Address, std::string& o_Name, std::string& o_Module )
{
    // Module
    std::size_t moduleBegin = a_Line.find_last_of("(");
    if( moduleBegin == std::string::npos )
        return false;
    o_Module = Replace(a_Line.substr(moduleBegin+1), ")", "");

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

//-----------------------------------------------------------------------------
void LinuxPerf::LoadPerfData( const std::string& a_FileName )
{
    SCOPE_TIMER_LOG(L"LoadPerfData");
    std::ifstream inFile(a_FileName);
    if( inFile.fail() )
    {
        PRINT_VAR("Could not open input file");
        PRINT_VAR(a_FileName);
        return;
    }

    LoadPerfData(inFile);
    inFile.close();
}

//-----------------------------------------------------------------------------
void LinuxPerf::LoadPerfData( std::istream& a_Stream )
{
    std::string header;
    uint32_t tid = 0;
    uint64_t time = 0;
    uint64_t numCallstacks = 0;

    CallStack CS;

    for (std::string line; std::getline(a_Stream, line); )
    {
        bool isHeader = !line.empty() && !StartsWith(line, "\t");
        bool isStackLine = !isHeader && !line.empty();
        bool isEndBlock = !isStackLine && line.empty() && !header.empty();

        if(isHeader)
        {
            header = line;
            auto tokens = Tokenize(line);
            time = tokens.size() > 2 ? GetMicros(tokens[2])*1000 : 0;
            tid  = tokens.size() > 1 ? atoi(tokens[1].c_str()) : 0;
        }
        else if(isStackLine)
        {
            std::string module;
            std::string function;
            uint64_t    address;

            if( !ParseStackLine( line, address, function, module ) )
            {
                PRINT_VAR("ParseStackLine error");
                PRINT_VAR(line);
                continue;
            }

            std::wstring moduleName = ToLower(Path::GetFileName(s2ws(module)));
            std::shared_ptr<Module> moduleFromName = Capture::GTargetProcess->GetModuleFromName( moduleName );

            // Debug - Temporary
            if( Contains(function, "Renderer::render") )
            {
                PRINT_VAR(function);
                PRINT_VAR(moduleName);
                PRINT_VAR(address);
            }

            if( moduleFromName )
            {
                uint64_t new_address = moduleFromName->ValidateAddress(address);

                // Debug - Temporary
                if( Contains(function, "Renderer::render") )
                {
                    std::cout << std::hex << address << " -> " << std::hex << new_address << std::endl;
                }
                address = new_address;
            }
            else
            {
                // Debug - Temporary
                if( Contains(function, "Renderer::render") ) {
                    PRINT_VAR("could not validate address");
                    PRINT_VAR(moduleName);
                }
            }

            CS.m_Data.push_back(address);

            if( Capture::GTargetProcess && !Capture::GTargetProcess->HasSymbol(address))
            {
                auto symbol = std::make_shared<LinuxSymbol>();
                symbol->m_Name = function;
                symbol->m_Module = module;
                Capture::GTargetProcess->AddSymbol( address, symbol );
            }
        }
        else if(isEndBlock)
        {
            if( CS.m_Data.size() )
            {
                CS.m_Depth = CS.m_Data.size();
                CS.m_ThreadId = tid;
                Capture::GSamplingProfiler->AddCallStack( CS );
                GEventTracer.GetEventBuffer().AddCallstackEvent( time, CS );
                ++numCallstacks;
            }

            CS.m_Data.clear();
            header = "";
            time = 0;
            tid = 0;
        }
    }

    PRINT_VAR(numCallstacks);
    PRINT_FUNC;
}