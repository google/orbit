#include <iostream>
#include <fstream>

#include "LinuxUtils.h"
#include "Utils.h"
#include "PrintVar.h"
#include "OrbitModule.h"
#include "Path.h"
#include "ScopeTimer.h"
#include "Capture.h"
#include "Callstack.h"
#include "SamplingProfiler.h"
#include "EventBuffer.h"
#include "Capture.h"
#include "ScopeTimer.h"
#include "OrbitProcess.h"

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

namespace LinuxUtils {

//-----------------------------------------------------------------------------
std::vector<std::string> ListModules( uint32_t a_PID )
{
    std::vector<std::string> modules;
    std::string result = ExecuteCommand( Format("cat /proc/%u/maps", a_PID).c_str() );

    std::stringstream ss(result);
    std::string line;
    while(std::getline(ss,line,'\n'))
    {
        modules.push_back(line);
    }

    return modules;
}

//-----------------------------------------------------------------------------
std::string ExecuteCommand( const char* a_Cmd ) 
{
    //SCOPE_TIMER_LOG(L"ExecuteCommand");
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);
    if (!pipe) {
        std::cout << "Could not open pipe" << std::endl;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

//-----------------------------------------------------------------------------
void ListModules( uint32_t a_PID, std::map< DWORD64, std::shared_ptr<Module> > & o_ModuleMap )
{
    std::map<std::string, std::shared_ptr<Module>> modules;
    std::vector<std::string> result = ListModules( a_PID );
    for( const std::string& line : result )
    {
        std::vector<std::string> tokens = Tokenize( line ); 
        if( tokens.size() == 6 )
        {
            std::string& moduleName = tokens[5];
            
            auto addresses = Tokenize(tokens[0], "-");
            if( addresses.size() == 2 )
            {
                auto iter = modules.find(moduleName);
                std::shared_ptr<Module> module = 
                    (iter == modules.end()) ? std::make_shared<Module>() : iter->second;

                uint64_t start = std::stoull(addresses[0], nullptr, 16);
                uint64_t end = std::stoull(addresses[1], nullptr, 16);
                
                if( module->m_AddressStart == 0 || start < module->m_AddressStart )
                    module->m_AddressStart = start;
                if( end > module->m_AddressEnd )
                    module->m_AddressEnd = end;
                
                module->m_FullName = s2ws(moduleName);
                module->m_Name = Path::GetFileName( module->m_FullName );;
                module->m_Directory = Path::GetDirectory( module->m_FullName );
                auto prettyName = module->GetPrettyName();
                modules[moduleName] = module;
            }
        }
    }

    for( auto& iter : modules )
    {
        auto module = iter.second;
        o_ModuleMap[module->m_AddressStart] = module;
    }
}

//-----------------------------------------------------------------------------
uint32_t GetPID(const char* a_Name)
{
    std::string command = Format("ps -A | grep %s", a_Name);
    std::string result = ExecuteCommand( command.c_str() );
    auto tokens = Tokenize(result);
    if( !tokens.empty() )
        return (uint32_t)atoi(tokens[0].c_str());
    std::cout << "Could not find process " << a_Name;
    return 0;
}

//-----------------------------------------------------------------------------
std::unordered_map<uint32_t, float> GetCpuUtilization()
{
    std::unordered_map<uint32_t, float> processMap;
    std::string result = ExecuteCommand("top -b -n 1 | sed -n '8, 1000{s/^ *//;s/ *$//;s/  */,/gp;};1000q'");
    std::stringstream ss(result);
    std::string line;

    while (std::getline(ss, line, '\n'))
    {
        auto tokens = Tokenize(line, ",");
        if( tokens.size() > 8)
        {
            uint32_t pid = atoi(tokens[0].c_str());
            float cpu = atof(tokens[8].c_str());
            processMap[pid] = cpu;
        }
    }

    return processMap;
}

//-----------------------------------------------------------------------------
bool Is64Bit( uint32_t a_PID )
{
    std::string result = ExecuteCommand(Format("file -L /proc/%u/exe", a_PID).c_str());
    return Contains(result, "64-bit");
}

//-----------------------------------------------------------------------------
std::string Demangle(const char* name)
{
    int status = 0;
    std::unique_ptr<char, void(*)(void*)> res {
        abi::__cxa_demangle(name, NULL, NULL, &status),
        std::free
    };

    return (status==0) ? res.get() : name;
}

//-----------------------------------------------------------------------------
void PrintBuffer( void* a_Buffer, size_t a_Size )
{
    unsigned char* buffer = (unsigned char*) a_Buffer;
    for (size_t i = 0; i < a_Size; ++i)
    {
        std::cout << std::hex 
                  << std::setfill('0') << std::setw(2) 
                  << (int)buffer[i] << " ";
        
        if( (i+1) % 32 == 0 )
        {
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
}

//-----------------------------------------------------------------------------
void StreamCommandOutput(const char* a_Cmd, std::function<void(const std::string&)> a_Callback, bool* a_ExitRequested)
{
    std::cout << "Starting output stream for command" << a_Cmd << std::endl;

    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(a_Cmd, "r"), pclose);
    
    if (!pipe) 
    {
        std::cout << "Could not open pipe" << std::endl;
    }
    
    while ( fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr && 
            !(*a_ExitRequested) )
    {
        a_Callback(buffer.data());
    }
    
    std::cout << "end stream" << std::endl;
}

}

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
    m_IsRunning = true;

    //std::string cmd = Format("record -F %u -p %u -g -o /tmp/perf.data", m_Frequency, m_PID);
    std::string cmd = "record -F 1000";
    std::string path = ws2s(Path::GetBasePath());
    
    pid_t PID = fork();
    if(PID == 0) {
        PRINT_VAR(cmd);
        PRINT_VAR(path);
        execl   ( "/usr/bin/perf"
                , "/usr/bin/perf"
                , "record"
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
}

//-----------------------------------------------------------------------------
void LinuxPerf::Stop()
{
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
        LoadPerfData(m_ReportFile);
    // });

    // m_Thread->detach();
}

//-----------------------------------------------------------------------------
void LinuxPerf::LoadPerfData( const std::string& a_FileName )
{
    std::ifstream infile(a_FileName.c_str());
    
    SCOPE_TIMER_LOG(L"LoadPerfData");
    std::ifstream inFile(a_FileName);
    if( inFile.fail() )
    {
        PRINT_VAR("Could not open input file");
        PRINT_VAR(a_FileName);
        return;
    }

    std::string header;
    uint32_t tid = 0;
    uint64_t time = 0;
    uint64_t numCallstacks = 0;

    CallStack CS;

    for (std::string line; std::getline(inFile, line); )
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
            auto tokens = Tokenize(line, " \t");
            if( tokens.size() == 3 )
            {
                uint64_t address = std::stoull(tokens[0], nullptr, 16);

                std::string moduleFullName = Replace(tokens[2], "(", "");
                moduleFullName = Replace(moduleFullName, ")", "");
                std::wstring moduleName = ToLower(Path::GetFileName(s2ws(moduleFullName)));
                std::shared_ptr<Module> moduleFromName = Capture::GTargetProcess->GetModuleFromName( moduleName );

                if( moduleFromName )
                {
                    address = moduleFromName->ValidateAddress(address);
                }

                CS.m_Data.push_back(address);

                if( Capture::GTargetProcess && !Capture::GTargetProcess->HasSymbol(address))
                {
                    auto symbol = std::make_shared<LinuxSymbol>();
                    symbol->m_Name = tokens[1];
                    symbol->m_Module = moduleFullName; 
                    Capture::GTargetProcess->AddSymbol( address, symbol );
                }
            }
            else
            {
                PRINT_FUNC;
                PRINT_VAR(line);
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

