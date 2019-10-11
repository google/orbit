//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include <iostream>
#include <fstream>

#include "LinuxUtils.h"
#include "Utils.h"
#include "PrintVar.h"
#include "Capture.h"
#include "CoreApp.h"
#include "Callstack.h"
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
#include "Params.h"
#include <regex>

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
    PRINT_FUNC;
#if __linux__
    if (!GParams.m_UseBPFTrace)
        AttachProbes();

    m_IsRunning = true;

    std::string path = ws2s(Path::GetBasePath());

    pid_t PID = fork();
    if(PID == 0) {
        std::vector<std::string> args;
        args.push_back("/usr/bin/perf");
        args.push_back("record");

        args.push_back("-k"); 
        args.push_back("monotonic");

        args.push_back("-g");

        args.push_back("-o");
        args.push_back(m_OutputFile);

        args.push_back("--event=cycles");
        
        args.push_back("-F");
        args.push_back(Format("%u", m_Frequency));

        args.push_back("-p");
        args.push_back(Format("%u", m_PID));
            
        if (!GParams.m_UseBPFTrace)
        {
            PRINT("Adding Probe Events To Perf");
            for (auto pair : Capture::GSelectedFunctionsMap) {
                Function *func = pair.second;
                assert(func->IsSelected());

                uint64_t func_Address = func->m_Address;
                std::string module_name = LinuxUtils::GetModuleBaseName(func->m_Module);
                std::stringstream probe_name;
                probe_name << "--event='{probe_" << module_name << ":abs_" << std::hex << func_Address <<",";
                probe_name << "probe_" << module_name << ":abs_" << std::hex << func_Address <<"__return}'";

                args.push_back(probe_name.str());
                args.push_back("-p");
                args.push_back(Format("%u", m_PID));
            }
        }

        std::vector<const char *> argsv;

        for (const auto& arg : args) {
            argsv.push_back(arg.c_str());
        }
        argsv.push_back(nullptr);

        execv( "/usr/bin/perf", (char* const*) argsv.data());
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
    PRINT_FUNC;
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

    if (!GParams.m_UseBPFTrace)
        DetachProbes();
    // });

    // m_Thread->detach();
#endif
}

void LinuxPerf::AttachProbes()
{
    #if __linux__
        PRINT_FUNC;
        std::stringstream ss;
        ss << "perf probe";
        for (auto pair : Capture::GSelectedFunctionsMap) {
            Function *func = pair.second;
            assert(func->IsSelected());

            uint64_t func_Address = func->m_Address;
            std::wstring module_file = func->m_Pdb->GetFileName();
            {
                ss << " -x " << ws2s(module_file) << " -a '0x" << std::hex << func_Address << "'";
                ss << " -x " << ws2s(module_file) << " -a '0x" << std::hex << func_Address << "%return'";
            }
        }  

        PRINT_VAR(Capture::GSelectedFunctionsMap.size());
        if (Capture::GSelectedFunctionsMap.empty())
            return ;

        std::string perf_command = ss.str();
        PRINT_VAR(perf_command);
        LinuxUtils::ExecuteCommand(perf_command.c_str());
    #endif
}

void LinuxPerf::DetachProbes()
{
    PRINT_FUNC;
    #if __linux__
        if (Capture::GSelectedFunctionsMap.empty())
            return;

        std::stringstream ss;
        ss << "perf probe";

        for (auto pair : Capture::GSelectedFunctionsMap) {
            Function *func = pair.second;
            assert(func->IsSelected());

            uint64_t func_Address = func->m_Address;
            std::string func_Module = func->m_Module;
            std::string module_name = LinuxUtils::GetModuleBaseName(func->m_Module);
            ss << " -d probe_" << module_name << ":abs_" << std::hex << func_Address;
            ss << " -d probe_" << module_name << ":abs_" << std::hex << func_Address << "__return";
        }

        std::string perf_command = ss.str();
        PRINT_VAR(perf_command);
        LinuxUtils::ExecuteCommand(perf_command.c_str());
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
    PRINT_FUNC;
    std::string header;
    uint32_t tid = 0;
    uint64_t time = 0;
    uint64_t numCallstacks = 0;
    std::string probe_name;
    uint64_t virtual_function_address = 0;
    bool isUProbeBegin = false;

    CallStack CS;
    std::map<uint32_t, std::vector<Timer>> timerStacks;

    // regex groups: comm, pid, tid?, time, binary_name, func_addr, return?
    const std::regex uprobe_regex("([^ ]+) ([0-9]+) \\[([0-9]+)\\][ \t]+([0-9\\.]+):[ \t]+probe_([^:]+):abs_([a-f0-9]+)(__return)?:[ \t]+\\(([a-f0-9]+)");
    std::smatch match;        
    
    for (std::string line; std::getline(a_Stream, line); )
    {
        bool isHeader = !line.empty() && !StartsWith(line, "\t");
        bool isStackLine = !isHeader && !line.empty();
        bool isEndBlock = !isStackLine && line.empty() && !header.empty();
        bool isUProbeEnd;

        if(isHeader)
        {
            header = line;
            if (std::regex_search(header, match, uprobe_regex))
            {
                // in case of a uprobe:
                isUProbeBegin = true;
                tid = std::stoi(match.str(2));
                time = GetMicros(match.str(4))*1000;
                isUProbeBegin = match.str(7).empty();
                isUProbeEnd = !isUProbeBegin;
                virtual_function_address = std::stoull(match.str(8), nullptr, 16);
                
                if (isUProbeBegin)
                {
                    Timer timer;
                    timer.m_TID = tid;
                    timer.m_Start = time;
                    timer.m_Depth = (uint8_t)timerStacks[tid].size();
                    timer.m_FunctionAddress = virtual_function_address;
                    timerStacks[tid].push_back(timer);
                }

                if (isUProbeEnd)
                {
                    std::vector<Timer>& timers = timerStacks[tid];
                    if (timers.size())
                    {
                        Timer& timer = timers.back();
                        timer.m_End = time;
                        GCoreApp->ProcessTimer(&timer, std::to_string(virtual_function_address));
                        timers.pop_back();
                    }
                }
            } else {
                // in case of a sampling event:
                isUProbeBegin = false;
                auto tokens = Tokenize(line);
                time = tokens.size() > 2 ? GetMicros(tokens[2])*1000 : 0;
                tid  = tokens.size() > 1 ? atoi(tokens[1].c_str()) : 0;
            }
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
            std::shared_ptr<Module> moduleFromName = Capture::GTargetProcess->GetModuleFromName( ws2s(moduleName) );

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
                CS.m_Depth = (uint32_t)CS.m_Data.size();
                CS.m_ThreadId = tid;
                if (isUProbeBegin)
                {
                    std::vector<Timer>& timers = timerStacks[tid];
                    if (timers.size())
                    {
                        Timer& timer = timers.back();
                        timer.m_CallstackHash = CS.Hash();
                        Capture::AddCallstack( CS );
                    }
                }
                else
                {
                    Capture::GSamplingProfiler->AddCallStack( CS );
                    GEventTracer.GetEventBuffer().AddCallstackEvent( time, CS );
                }
                ++numCallstacks;
            }

            CS.m_Data.clear();
            header = "";
            time = 0;
            tid = 0;
            probe_name = "";
            virtual_function_address = 0;
            isUProbeBegin = false;
        }
    }

    PRINT_VAR(numCallstacks);
    PRINT_FUNC;
}